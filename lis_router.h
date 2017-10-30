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

#ifndef LIS_ROUTER_H
#define LIS_ROUTER_H 

#define LIS_ROUTER_PROPERTY_NAME_INSTANCE                   "_instance"
#define LIS_ROUTER_PROPERTY_NAME_GROUP_ROUTE                "_group_route"
#define LIS_ROUTER_PROPERTY_NAME_ROUTER                     "_router"
#define LIS_ROUTER_PROPERTY_NAME_PARAMS_ROUTER              "_params_router"
#define LIS_ROUTER_PROPERTY_NAME_PARAMS_STR_GROUP_ROUTER    "_params_str_group_router"

#define LIS_DELIMITER_LEFT      '{'
#define LIS_DELIMITER_RIGHT     '}'
#define LIS_DELIMITER_MIDDLE    ':'

#define LIS_ROUTER_METHOD_QUERY(x) \
    do { \
        char *uri; \
        if (LIS_G(hit_router) == 1) { \
            RETURN_TRUE; \
        } \
        zend_string *match= NULL, *func_name = NULL; \
        if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "SS", &match, &func_name) == FAILURE) { \
            RETURN_FALSE; \
        } \
        uri = LIS_G(base_uri) ? ZSTR_VAL(LIS_G(base_uri)) : NULL; \
        if (uri == NULL) { \
            RETURN_FALSE; \
        } \
        zval *method; \
        method = lis_request_get_method(); \
        if (strcmp(Z_STRVAL_P(method), #x) != 0) { \
            zend_string_release(match); \
            zend_string_release(func_name); \
            RETURN_FALSE; \
        } \
        lis_check_router_params(match, func_name, uri); \
        if (LIS_G(hit_router) == 1) { \
            lis_request_load_controller(1); \
        } \
        zend_string_release(match); \
        zend_string_release(func_name); \
        RETURN_TRUE; \
    } while(0)

extern zend_class_entry *lis_router_ce;
void lis_router_instance();

static int lis_check_group_router_params(zend_string *group_router_name, char *uri);
static int lis_check_route_pcre(char *route, char *uri, zval *res_route_params, int is_group);
static void lis_set_group_router_null();
int lis_check_router_params(zend_string *router_name, zend_string *func_name, char *uri);
static char *lis_extraction_regularity(char *route_name, char *route_params);

void lis_router_check_match(char *regex, char *uri);
void lis_group_router_params(zend_string *name, char *uri);
void lis_router_params(zend_string *name, zend_string *zmatch, char *uri, zval *res_router, zval *res_router_params);
static void lis_set_controller_action(zend_string *func_name);

LIS_STARTUP_FUNCTION(router);

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
