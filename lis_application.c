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

#include "php_lis.h"
#include "lis_loader.h"
#include "lis_router.h"
#include "lis_container.h"
#include "lis_request.h"
#include "lis_exception.h"
#include "lis_application.h"

zend_class_entry *lis_application_ce;

/* {{{ ARG_INFO
 */
ZEND_BEGIN_ARG_INFO_EX(lis_application_void_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_application_construct_arginfo, 0, 0, 0)
    ZEND_ARG_ARRAY_INFO(0, container, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_application_set_app_directory_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, directory)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_application_get_environ_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_application_get_app_directory_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ proto LisApplication::setAppDirectory(string $directory)
 */
PHP_METHOD(lis_application, setAppDirectory) 
{
    zend_string *directory;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "S", &directory) == FAILURE) {
        RETURN_FALSE;
    }

    LIS_G(directory) = zend_string_copy(directory);
    RETURN_TRUE;
}
/* }}} */

/* {{{ proto LisApplication::run()
 */
PHP_METHOD(lis_application, run) 
{
    zval *method;
    method = lis_request_get_method();
    if (LIS_G(hit_action) == 0 && strcmp(Z_STRVAL_P(method), "Cli")) {
        lis_throw_exception(LIS_ERR_CONTROLLER_FAILED, "Controller class does not exist");
    }
    RETURN_TRUE;
}
/* }}} */

/* {{{ proto LisApplication::construct(array $container)
 */
PHP_METHOD(lis_application, __construct) 
{
    zval *container;
    lis_application_t *self;
    lis_container_t zcontainer = {{0}};
    zval *run;

    self = getThis();

    run = zend_read_property(lis_application_ce, self, ZEND_STRL(LIS_APPLICATION_PROPERTY_NAME_RUN), 1, NULL);    
    if (UNEXPECTED(Z_TYPE_P(run) == IS_TRUE)) {
        RETURN_TRUE;
    }

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "|a", &container) == FAILURE) {
        RETURN_FALSE;
    }

    zend_update_property_bool(lis_application_ce, self, ZEND_STRL(LIS_APPLICATION_PROPERTY_NAME_RUN), 1);

    lis_container_add(container);

    //Ìí¼ÓÄ¬ÈÏÈÝÆ÷
    lis_container_instance(&zcontainer);
    zend_call_method_with_0_params(&zcontainer, Z_OBJCE_P(&zcontainer), NULL, "__construct", NULL);
    zval_ptr_dtor(&zcontainer);
    RETURN_TRUE;

}
/* }}} */

/* {{{ proto LisApplication::getEnviron()
 */
PHP_METHOD(lis_application, getEnviron)
{
    RETURN_STRING(LIS_G(environ));
}
/* }}} */

/* {{{ proto LisApplication::getAppDirectory()
 */
PHP_METHOD(lis_application, getAppDirectory) 
{
    RETURN_STRING(ZSTR_VAL(LIS_G(directory)));
}
/* }}} */

/* {{{ lisApplication_methods
 */
zend_function_entry lis_application_methods[] = {
    PHP_ME(lis_application, __construct, lis_application_construct_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(lis_application, run, lis_application_void_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(lis_application, getEnviron, lis_application_get_environ_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(lis_application, setAppDirectory, lis_application_set_app_directory_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(lis_application, getAppDirectory, lis_application_get_app_directory_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ LIS_STARTUP_FUNCTION
*/
LIS_STARTUP_FUNCTION(application) {
    zend_class_entry ce;

    LIS_INIT_CLASS_ENTRY(ce, "LisApplication", "Lis\\Application", lis_application_methods);
    lis_application_ce = zend_register_internal_class_ex(&ce, NULL);
    lis_application_ce->ce_flags |= ZEND_ACC_FINAL;

    zend_declare_property_bool(lis_application_ce, ZEND_STRL(LIS_APPLICATION_PROPERTY_NAME_RUN), 0, ZEND_ACC_PROTECTED);
    zend_declare_property_string(lis_application_ce, ZEND_STRL(LIS_APPLICATION_PROPERTY_NAME_ENV), LIS_G(environ), ZEND_ACC_PROTECTED);

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


