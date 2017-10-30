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

#include "php_lis.h"
#include "lis_config.h"
#include "lis_exception.h"
#include "lis_application.h"

zend_class_entry *lis_config_ce;

/* {{{ ARG_INFO
 */
ZEND_BEGIN_ARG_INFO_EX(lis_config_void_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_config_set_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, name)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_config_get_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_config_load_arginfo, 0, 0, 1)
    ZEND_ARG_ARRAY_INFO(0, config, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_config_getinfo_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

/* }}} */

static void lis_config_zval_copy_ctor(zval *p) /* {{{ */
{
    zval_copy_ctor(p);
}
/* }}} */

void lis_config_instance() /* {{{ */
{
    lis_config_t *instance;
    instance = zend_read_static_property(lis_config_ce, ZEND_STRL(LIS_CONFIG_PROPERTY_NAME_INSTANCE), 1);

    if (UNEXPECTED(IS_OBJECT == Z_TYPE_P(instance))) {
        return;
    }

    object_init_ex(instance, lis_config_ce);

    zval zconfig;
    array_init(&zconfig);

    zend_update_static_property(lis_config_ce, ZEND_STRL(LIS_CONFIG_PROPERTY_NAME_INSTANCE), instance);
    zend_update_static_property(lis_config_ce, ZEND_STRL(LIS_CONFIG_PROPERTY_NAME_CONFIG), &zconfig);
    zval_ptr_dtor(&zconfig);

    return;
}
/* }}} */

/* {{{ proto Lisconfig::construct()
 */
PHP_METHOD(lis_config, __construct) 
{ 
    return;
}
/* }}} */

/* {{{ proto LisConfig::get()
 */
PHP_METHOD(lis_config, get) 
{
    zend_string *name; 
    zval *zconfig;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "S", &name) == FAILURE) {
        RETURN_FALSE;
    }
    zconfig = zend_read_static_property(lis_config_ce, ZEND_STRL(LIS_CONFIG_PROPERTY_NAME_CONFIG), 1);

    if (strchr(ZSTR_VAL(name), '.')) {
        char *entry, *seg, *pptr;
        zend_long lval;
        double dval;
        zval *pzval;
        int seg_len;
        entry = estrndup(ZSTR_VAL(name), ZSTR_LEN(name));
        if ((seg = php_strtok_r(entry, ".", &pptr))) {
            while (seg) {
                seg_len = strlen(seg);
                if (is_numeric_string(seg, seg_len, &lval, &dval, 0) != IS_LONG) {
                    if ((pzval = zend_hash_str_find(Z_ARRVAL_P(zconfig), seg, seg_len)) == NULL) {
                        efree(entry);
                        RETURN_NULL();
                    }
                } else {
                    if ((pzval = zend_hash_index_find(Z_ARRVAL_P(zconfig), lval)) == NULL) {
                        efree(entry);
                        RETURN_NULL();
                    }
                }
                zconfig = pzval;
                seg = php_strtok_r(NULL, ".", &pptr);
            }

            efree(entry);
            RETURN_ZVAL(pzval, 1, 0);
        }
        efree(entry);
    } else if(zend_hash_exists(Z_ARRVAL_P(zconfig), name) == 1) {
        zval *zret;
        zret = zend_hash_find(Z_ARRVAL_P(zconfig), name);
        RETURN_ZVAL(zret, 1, 0);
    }
    RETURN_NULL();
}
/* }}} */

/* {{{ proto LisConfig::load($config)
 */
PHP_METHOD(lis_config, load)
{
    zval *value, *zconfig;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "a", &value) == FAILURE) {
        RETURN_FALSE;
    }

    zconfig = zend_read_static_property(lis_config_ce, ZEND_STRL(LIS_CONFIG_PROPERTY_NAME_CONFIG), 1);
    zend_hash_merge(Z_ARRVAL_P(zconfig), Z_ARRVAL_P(value), lis_config_zval_copy_ctor, 0); 

    RETURN_TRUE;
}
/* }}} */

/* {{{ proto LisConfig::set($name, $value)
 */
PHP_METHOD(lis_config, set) 
{
    zend_string *name;
    zval *value, *zconfig;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "Sz", &name, &value) == FAILURE) {
        RETURN_FALSE;
    }
    zconfig = zend_read_static_property(lis_config_ce, ZEND_STRL(LIS_CONFIG_PROPERTY_NAME_CONFIG), 1);
    
    if (zend_hash_update(Z_ARRVAL_P(zconfig), name, value) != NULL) {
        Z_TRY_ADDREF_P(value);
        RETURN_TRUE;
    }

    RETURN_FALSE;
}
/* }}} */

/* {{{ lis_config_methods
*/
zend_function_entry lis_config_methods[] = {
    PHP_ME(lis_config, __construct, lis_config_void_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(lis_config, load, lis_config_load_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(lis_config, set, lis_config_set_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(lis_config, get, lis_config_getinfo_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ LIS_STARTUP_FUNCTION
*/
LIS_STARTUP_FUNCTION(config) {
    zend_class_entry ce;

    LIS_INIT_CLASS_ENTRY(ce, "LisConfig", "Lis\\Config", lis_config_methods);
    lis_config_ce = zend_register_internal_class_ex(&ce, NULL);
    lis_config_ce->ce_flags |= ZEND_ACC_FINAL;

    zend_declare_property_null(lis_config_ce, ZEND_STRL(LIS_CONFIG_PROPERTY_NAME_INSTANCE),  ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
    zend_declare_property_null(lis_config_ce, ZEND_STRL(LIS_CONFIG_PROPERTY_NAME_CONFIG),  ZEND_ACC_PUBLIC|ZEND_ACC_STATIC);

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


