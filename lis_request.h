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

#ifndef LIS_REQUEST_H
#define LIS_REQUEST_H

#define LIS_REQUEST_PROPERTY_NAME_INSTANCE          "_instance"
#define LIS_REQUEST_PROPERTY_NAME_CONTROLLER_NAME   "_controller_name"
#define LIS_REQUEST_PROPERTY_NAME_ACTION_NAME       "_action_name"
#define LIS_REQUEST_PROPERTY_NAME_METHOD            "_method"
#define LIS_REQUEST_PROPERTY_NAME_BASE_URI          "_base_uri"

static zval *lis_request_query(uint type, zend_bool str_type, void *name, size_t len);

extern zend_class_entry *lis_request_ce;
void lis_request_instance();
void lis_request_load_controller(int is_init);
void lis_request_set_controller_action();
zval *lis_request_get_method();

LIS_STARTUP_FUNCTION(request);

#define LIS_REQUEST_PARAMS_METHOD(ce, name, var_name) \
PHP_METHOD(ce, get##name) { \
    zval *ret; \
    ret = zend_read_static_property(lis_request_ce, ZEND_STRL(var_name), 1); \
    RETURN_ZVAL(ret, 1, 0); \
}

#define LIS_REQUEST_METHOD(ce, name, type) \
PHP_METHOD(ce, get##name) { \
    zend_string *name; \
    zval *ret; \
    zval *def = NULL; \
    if (ZEND_NUM_ARGS() == 0) { \
        ret = lis_request_query(type, 1, NULL, 0); \
    } else if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|z", &name, &def) == FAILURE) { \
        return; \
    } else { \
        ret = lis_request_query(type, 1, name, 0); \
        if (!ret && def != NULL) { \
            RETURN_ZVAL(def, 1, 0); \
        } \
    } \
    if (ret) { \
        RETURN_ZVAL(ret, 1, 0); \
    } else { \
        RETURN_NULL(); \
    } \
} 

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
