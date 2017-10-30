/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2017 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: shendegang <php_shen@163.com>                                |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "Zend/zend_interfaces.h"
#include "Zend/zend_exceptions.h"

#include "php_lis.h"
#include "lis_exception.h"
#include "lis_application.h"

zend_class_entry *lis_exception_ce;
zend_class_entry *lis_sys_exceptions[LIS_MAX_SYS_EXCEPTION];

/* {{{ ARG_INFO
 */
ZEND_BEGIN_ARG_INFO_EX(lis_exception_void_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_lis_set_exception_handler, 0, 0, 1)
    ZEND_ARG_INFO(0, exception_handler)
ZEND_END_ARG_INFO()
/* }}} */

void lis_throw_exception(int code, char *format, ...) /* {{{ */
{
    va_list args;
    char *message;
    uint msg_len;

    va_start(args, format);
    msg_len = vspprintf(&message, 0, format, args);
    va_end(args);

    zend_class_entry *base_exception = lis_exception_ce;

    if ((code & LIS_ERR_BASE) == LIS_ERR_BASE && lis_sys_exceptions[LIS_EXCEPTION_OFFSET(code)]) {
        base_exception = lis_sys_exceptions[LIS_EXCEPTION_OFFSET(code)];
    }
    zend_throw_exception(base_exception, message, code);

    efree(message);
}
/* }}} */

void lis_exception_instance() /* {{{ */
{
    lis_exception_t *instance;
    instance = zend_read_static_property(lis_exception_ce, ZEND_STRL(LIS_EXCEPTION_PROPERTY_NAME_INSTANCE), 1);

    if (UNEXPECTED(IS_OBJECT == Z_TYPE_P(instance))) {
        return;
    }

    object_init_ex(instance, lis_exception_ce);

    zend_update_static_property(lis_exception_ce, ZEND_STRL(LIS_EXCEPTION_PROPERTY_NAME_INSTANCE), instance);

    return;
}
/* }}} */

/* {{{ proto LisException::setExceptionHandler(array $exception_handler)
 */
PHP_METHOD(lis_exception, setExceptionHandler)
{

    zval *exception_handler;
    zend_string *exception_handler_name = NULL;
    zval *controller_name;
    zval *action_name;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "|z", &exception_handler) == FAILURE) {
        return;
    }

    if (Z_TYPE_P(exception_handler) != IS_NULL) { 

        controller_name = zend_hash_index_find(Z_ARRVAL_P(exception_handler), 0);
        action_name = zend_hash_index_find(Z_ARRVAL_P(exception_handler), 1);
        lis_request_set_controller_action(Z_STRVAL_P(controller_name), Z_STRVAL_P(action_name));
        lis_request_load_controller(0);

        if (!zend_is_callable(exception_handler, 0, &exception_handler_name)) {
            zend_error(E_WARNING, "%s() expects the argument (%s) to be a valid callback",
                       get_active_function_name(), exception_handler_name?ZSTR_VAL(exception_handler_name):"unknown");
            zend_string_release(exception_handler_name);
            return;
        }
        zend_string_release(exception_handler_name);

    }

    if (Z_TYPE(EG(user_exception_handler)) != IS_UNDEF) {
        ZVAL_COPY(return_value, &EG(user_exception_handler));

        zend_stack_push(&EG(user_exception_handlers), &EG(user_exception_handler));
    }

    if (Z_TYPE_P(exception_handler) == IS_NULL) {
        ZVAL_UNDEF(&EG(user_exception_handler));
        return;
    }

    ZVAL_COPY(&EG(user_exception_handler), exception_handler);
} 
/* }}} */

/* {{{ proto LisException::__construct()
 */
PHP_METHOD(lis_exception, __construct)
{
    return;
}
/* }}} */

/* {{{ lis_exception_methods
*/
zend_function_entry lis_exception_methods[] = {
    PHP_ME(lis_exception, __construct, lis_exception_void_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(lis_exception, setExceptionHandler, arginfo_lis_set_exception_handler, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ LIS_STARTUP_FUNCTION
 */
LIS_STARTUP_FUNCTION(exception) 
{
    zend_class_entry ce;
    zend_class_entry container_ce;

    LIS_INIT_CLASS_ENTRY(ce, "LisException", "Lis\\Exception", lis_exception_methods);
    lis_exception_ce = zend_register_internal_class_ex(&ce, zend_exception_get_default());

    zend_declare_property_null(lis_exception_ce, ZEND_STRL(LIS_EXCEPTION_PROPERTY_NAME_INSTANCE),  ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
    zend_declare_property_null(lis_exception_ce, ZEND_STRL("message"), ZEND_ACC_PROTECTED);
    zend_declare_property_long(lis_exception_ce, ZEND_STRL("code"), 0, ZEND_ACC_PROTECTED);
    zend_declare_property_null(lis_exception_ce, ZEND_STRL("previous"), ZEND_ACC_PROTECTED);

    INIT_CLASS_ENTRY(container_ce, "LisExceptionControllerFailed", NULL);
    lis_sys_exceptions[LIS_EXCEPTION_OFFSET(LIS_ERR_CONTROLLER_FAILED)] = zend_register_internal_class_ex(&container_ce, lis_exception_ce);

    INIT_CLASS_ENTRY(container_ce, "LisExceptionMiddlewareFailed", NULL);
    lis_sys_exceptions[LIS_EXCEPTION_OFFSET(LIS_ERR_MIDDLEWARE_FAILED)] = zend_register_internal_class_ex(&container_ce, lis_exception_ce);

    INIT_CLASS_ENTRY(container_ce, "LisExceptionViewFailed", NULL);
    lis_sys_exceptions[LIS_EXCEPTION_OFFSET(LIS_ERR_VIEW_FAILED)] = zend_register_internal_class_ex(&container_ce, lis_exception_ce);

    return SUCCESS;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */


