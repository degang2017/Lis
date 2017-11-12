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
#include "lis_view.h"
#include "lis_router.h"
#include "lis_loader.h"
#include "lis_container.h"
#include "lis_exception.h"
#include "lis_application.h"

zend_class_entry *lis_container_ce;

/* {{{ ARG_INFO
 */
ZEND_BEGIN_ARG_INFO_EX(lis_container_void_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_container_get_val_arginfo, 0, 0, 0)
    ZEND_ARG_INFO(0, container_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_container_add_val_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, container_name)
    ZEND_ARG_INFO(0, container)
ZEND_END_ARG_INFO()
/* }}} */

lis_container_t *lis_container_instance(lis_container_t *this_ptr) /* {{{ */
{
    lis_container_t *instance;
    instance = zend_read_static_property(lis_container_ce, ZEND_STRL(LIS_CONTAINER_PROPERTY_NAME_INSTANCE), 1);

    if (UNEXPECTED(IS_OBJECT == Z_TYPE_P(instance))) {
        return instance;
    }

    if (EXPECTED(Z_ISUNDEF_P(this_ptr))) {
        object_init_ex(this_ptr, lis_container_ce);
    }

    zend_update_static_property(lis_container_ce, ZEND_STRL(LIS_CONTAINER_PROPERTY_NAME_INSTANCE), this_ptr);

    return this_ptr;
}
/* }}} */

void lis_container_add(zval *container) /* {{{ */
{
    zval e_container;
    if (IS_FALSE == Z_TYPE_P(container) || IS_UNDEF == Z_TYPE_P(container) || IS_NULL == Z_TYPE_P(container)) {
        array_init(&e_container);
        zend_update_static_property(lis_container_ce, ZEND_STRL(LIS_CONTAINER_PROPERTY_NAME_CONTAINER), &e_container);
        zval_ptr_dtor(&e_container);
    } else {
        zend_update_static_property(lis_container_ce, ZEND_STRL(LIS_CONTAINER_PROPERTY_NAME_CONTAINER), container);
    }
}
/* }}} */

/* {{{ proto LisContainer::get(string $container_name)
 */
PHP_METHOD(lis_container, get)
{
    zend_string *container_name = NULL;

    zval *container, *container_obj;
    lis_application_t *self;

    self = getThis();

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "|S", &container_name) == FAILURE) {
        return; 
    }

    container = zend_read_static_property(lis_container_ce, ZEND_STRL(LIS_CONTAINER_PROPERTY_NAME_CONTAINER), 1);

    if (IS_UNDEF == Z_TYPE_P(container)) {
        array_init(container);
    }

    if (container_name == NULL) {
        zval ret_val;
        ZVAL_COPY(&ret_val, container);
        RETURN_ARR(Z_ARRVAL_P(&ret_val));
    }

    if (zend_hash_exists(Z_ARRVAL_P(container), container_name) == 1) {
        container_obj = zend_hash_find(Z_ARRVAL_P(container), container_name);
        RETURN_ZVAL(container_obj, 1, 0);
    }

    return;
} 
/* }}} */

/* {{{ proto Liscontainer::construct()
 */
PHP_METHOD(lis_container, __construct) 
{
    lis_config_t zconfig = {{0}};
    lis_exception_t zexception = {{0}};
    lis_router_t zrouter = {{0}};
    lis_loader_t zloader = {{0}};
    lis_request_t zrequest = {{0}};
    lis_view_t zview = {{0}};

    //config
    lis_config_instance();

    //exception
    lis_exception_instance();

    //router
    lis_router_instance();

    //loader
    lis_loader_instance();

    //view
    lis_view_instance();

    //request
    lis_request_instance();

    RETURN_TRUE;
}
/* }}} */

/* {{{ proto LisContainer::add(string $container_name, object $container)
 */
PHP_METHOD(lis_container, add)
{
    zval *container;
    zend_string *container_name;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "Sz", &container_name, &container) == FAILURE) {
        RETURN_FALSE;
    }

    zval *container_z = zend_read_static_property(lis_container_ce, ZEND_STRL(LIS_CONTAINER_PROPERTY_NAME_CONTAINER), 1);
    
    if (zend_hash_exists(Z_ARRVAL_P(container_z), container_name) == 0) {
        zval container_zz;
        ZVAL_COPY(&container_zz, container);
        add_assoc_zval(container_z, ZSTR_VAL(container_name), &container_zz);
    }

    zend_update_static_property(lis_container_ce, ZEND_STRL(LIS_CONTAINER_PROPERTY_NAME_CONTAINER), container_z);

    RETURN_TRUE;
}
/* }}} */

/* {{{ lis_container_methods
*/
zend_function_entry lis_container_methods[] = {
    PHP_ME(lis_container, __construct, lis_container_void_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(lis_container, get, lis_container_get_val_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(lis_container, add, lis_container_add_val_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ LIS_STARTUP_FUNCTION
*/
LIS_STARTUP_FUNCTION(container) 
{
    zend_class_entry ce;

    LIS_INIT_CLASS_ENTRY(ce, "LisContainer", "Lis\\Container", lis_container_methods);
    lis_container_ce = zend_register_internal_class_ex(&ce, NULL);
    lis_container_ce->ce_flags |= ZEND_ACC_FINAL;

    zend_declare_property_null(lis_container_ce, ZEND_STRL(LIS_CONTAINER_PROPERTY_NAME_INSTANCE),  ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
    zend_declare_property_null(lis_container_ce, ZEND_STRL(LIS_CONTAINER_PROPERTY_NAME_CONTAINER), ZEND_ACC_STATIC | ZEND_ACC_PROTECTED);
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


