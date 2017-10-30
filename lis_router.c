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
#include "standard/php_string.h"
#include "Zend/zend_interfaces.h"
#include "zend_smart_str.h"
#include "php_ini.h"
#include "ext/pcre/php_pcre.h"

#include "php_lis.h"
#include "lis_loader.h"
#include "lis_request.h"
#include "lis_router.h"
#include "lis_exception.h"
#include "lis_application.h"

zend_class_entry *lis_router_ce;

/* {{{ ARG_INFO
 */
ZEND_BEGIN_ARG_INFO_EX(lis_router_void_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_router_group_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, match)
    ZEND_ARG_INFO(0, func)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_router_query_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, match)
    ZEND_ARG_INFO(0, func)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_router_middleware_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, middle_name)
ZEND_END_ARG_INFO()
/* }}} */

void lis_router_instance() /* {{{ */
{
    lis_router_t *instance;
    instance = zend_read_static_property(lis_router_ce, ZEND_STRL(LIS_ROUTER_PROPERTY_NAME_INSTANCE), 1);

    if (UNEXPECTED(IS_OBJECT == Z_TYPE_P(instance))) {
        return;
    }

    object_init_ex(instance, lis_router_ce);

    zend_update_static_property(lis_router_ce, ZEND_STRL(LIS_ROUTER_PROPERTY_NAME_INSTANCE), instance);

    return;
}
/* }}} */

static char *lis_extraction_regularity(char *router_name, char *router_params) /* {{{ 提取路由正则 */
{
    char *router_name_base = emalloc(strlen(router_name) + 4);

    int i = 2,j = 0, is_jump = 0;
    router_name_base[0] = '#';
    router_name_base[1] = '^';
    while (*router_name) {
        if (LIS_DELIMITER_LEFT  == *router_name) {
            router_name_base[i] = '(';
            ++i;
            is_jump = 1;
        }
        if (is_jump == 1) {
            if (LIS_DELIMITER_LEFT != *router_name) {
                router_params[j] = *router_name;
                ++j;
            }
        }

        if (LIS_DELIMITER_MIDDLE == *router_name) {
            ++router_name;
            is_jump = 0;
        }

        if (LIS_DELIMITER_RIGHT == *router_name) {
            ++router_name;
            router_name_base[i] = ')';
            ++i;
            continue;
        }

        if (is_jump == 1) {
            ++router_name;
            continue;
        }

        router_name_base[i] = *router_name;

        ++i;
        ++router_name;
    }
    router_name_base[i++] = '#';
    router_name_base[i++] = '\0';
    router_params[j] = '\0';
    return router_name_base;
}
/* }}} */

static int lis_check_route_pcre(char *router, char *uri, zval *res_router_params, int is_group) /* {{{ 路由匹配 */
{
    pcre_cache_entry *pce;

    if (strcmp(router, "#^/#") == 0) {
        if (strcmp(uri, "/") == 0) {
            array_init(res_router_params);
            return 0;
        } 
        return -1;
    }

    zval match_long;
    zend_string *zroute;
    zroute = zend_string_init(router, strlen(router), 0);

    if ((pce = pcre_get_compiled_regex_cache(zroute)) == NULL) {
        zend_string_release(zroute);
        return -1;
    }
    
    php_pcre_match_impl(pce, uri, strlen(uri), &match_long, res_router_params, 0, 0, 0, 0);

    zend_string_release(zroute);
    if (Z_LVAL_P(&match_long) > 0) {
        zval_ptr_dtor(&match_long);
        return 0;
    }

    zval_ptr_dtor(&match_long);
    return -1;
}
/* }}} */

static void lis_set_group_router_null() /* {{{ 设置组的路由为null */ 
{
    zend_update_static_property_null(lis_router_ce, ZEND_STRL(LIS_ROUTER_PROPERTY_NAME_GROUP_ROUTE));
}
/* }}} */

int lis_check_router_params(zend_string *router_name, zend_string *func_name, char *uri) /* {{{ 检查路由 */
{
    //判断组路由是否存在
    zval *group_router_name;
    zval res_router_params = {{0}};
    char *router_base_name;
    int i = 0,j = 0;
    int group_params_str_length = 0;
    int is_router_condition = 0;
    int group_router_name_length = 0; 
    char *cgroup_router_name = NULL;
    char *completeness_router = NULL;
    HashTable *ht;

    group_router_name = zend_read_static_property(lis_router_ce, ZEND_STRL(LIS_ROUTER_PROPERTY_NAME_GROUP_ROUTE), 1);
    int router_params_length = strlen(ZSTR_VAL(router_name)) + 1;
    char *router_params = emalloc(router_params_length);
    memset(router_params, '\0', router_params_length);
    
    //成员路由提取
    router_base_name = lis_extraction_regularity(ZSTR_VAL(router_name), router_params);                 

    if (!Z_ISNULL_P(group_router_name)) {
        group_router_name_length = Z_STRLEN_P(group_router_name);
    }
    int router_base_name_length = strlen(router_base_name);

    int router_length = (group_router_name_length + router_base_name_length + 1);
    completeness_router = emalloc(router_length);
    memset(completeness_router, '\0', router_length);

    if (!Z_ISNULL_P(group_router_name)) {
        cgroup_router_name = Z_STRVAL_P(group_router_name);
    }

    if (router_length > 0) {
        if (cgroup_router_name != NULL) {
            while (*cgroup_router_name) {
                if (i == (group_router_name_length-1)) {
                    break;
                }
                completeness_router[i] = *cgroup_router_name;
                ++cgroup_router_name;
                ++i;
            }
        } 

        while(*router_base_name) {
            if (j >= router_base_name_length) {
                break;
            }
            if (cgroup_router_name == NULL) {
                completeness_router[i] = *(router_base_name+j); 
            }
            if (j >= 0x02 && cgroup_router_name != NULL) {
                completeness_router[i-2] = *(router_base_name+j); 
            } 
        
            ++i;
            ++j;
        }

        completeness_router[i] = '\0';

        efree(router_base_name);

        //提取参数
        if (lis_check_route_pcre(completeness_router, uri, &res_router_params, 0) == 0) {

            zval router_arr_params = {{0}};
            zval *val, *group_router_params_str;
            ulong idx;
            is_router_condition = 1;
            LIS_G(hit_router) = 1;

            //参数字符串拼接
            group_router_params_str = zend_read_static_property(lis_router_ce, ZEND_STRL(LIS_ROUTER_PROPERTY_NAME_PARAMS_STR_GROUP_ROUTER), 1);
            if (Z_ISNULL_P(group_router_params_str)) {
                ZVAL_STRING(group_router_params_str, "");
            }
            group_params_str_length = Z_STRLEN_P(group_router_params_str) + strlen(router_params) + 1;

            char *router_params_str = emalloc(group_params_str_length);
            snprintf(router_params_str, group_params_str_length, "%s%s", Z_STRVAL_P(group_router_params_str), router_params);

            //将字符串转成数组
            zval params_arr;
            array_init(&params_arr);

            char *router_params_str_x = emalloc(strlen(router_params_str) + 1);
            i = 0;
            j = 0;
            int key_count = 0;
            while (*(router_params_str+i)) {
                router_params_str_x[j] = *(router_params_str+i);
                if (':' == *(router_params_str+i)) {
                    router_params_str_x[j] = '\0';
                    add_index_string(&params_arr, key_count, router_params_str_x);
                    j = -1;
                    ++key_count;
                }
                ++i;
                ++j;
            }

            //参数key 和 value 合并
            ht = Z_ARRVAL_P(&res_router_params);

            zval *valkey;
            zval new_val = {{0}};
            ZEND_HASH_FOREACH_NUM_KEY_VAL(ht, idx, val) {
                if (idx > 0) {
                    valkey = zend_hash_index_find(Z_ARRVAL_P(&params_arr), idx-1);

                    ZVAL_STRING(&new_val, Z_STRVAL_P(val));
                    zend_hash_str_add(Z_ARRVAL(PG(http_globals)[TRACK_VARS_GET]), Z_STRVAL_P(valkey), Z_STRLEN_P(valkey), &new_val);
                }

            } ZEND_HASH_FOREACH_END();

            //设置控制器及方法
            lis_set_controller_action(func_name);

            efree(router_params_str);
            efree(router_params_str_x);
            zval_ptr_dtor(&params_arr);

        }
    }

    zval_ptr_dtor(&res_router_params);
    efree(completeness_router);
    efree(router_params);

    if (is_router_condition == 1) {
        return 0;
    }
    return -1;
}
/* }}} */

static void lis_set_controller_action(zend_string *func_name) /* {{{ */
{
    char *controller, *entry, *action;
    char *name = ZSTR_VAL(func_name);
    entry = estrndup(name, strlen(name));
    controller = php_strtok_r(entry, ":", &action);

    zend_update_static_property_string(lis_request_ce, ZEND_STRL(LIS_REQUEST_PROPERTY_NAME_CONTROLLER_NAME), controller);
    zend_update_static_property_string(lis_request_ce, ZEND_STRL(LIS_REQUEST_PROPERTY_NAME_ACTION_NAME), action);
    efree(entry);
}
/* }}} */

static int lis_check_group_router_params(zend_string *group_router_name, char *uri) /* {{{ */
{

    char *find_delimiter;
    char *group_router_base_name;
    int group_router_name_length = strlen(ZSTR_VAL(group_router_name)) + 0x03;
    int is_group_condition = 0;

    //正则路由
    if ((find_delimiter = strrchr(ZSTR_VAL(group_router_name), LIS_DELIMITER_LEFT)) != NULL) {
        char *router_params = emalloc(strlen(ZSTR_VAL(group_router_name)) + 1);
        group_router_base_name = lis_extraction_regularity(ZSTR_VAL(group_router_name), router_params);                 

        if (lis_check_route_pcre(group_router_base_name, uri, NULL, 1) == 0) {
            is_group_condition = 1;
            zend_update_static_property_string(lis_router_ce, ZEND_STRL(LIS_ROUTER_PROPERTY_NAME_PARAMS_STR_GROUP_ROUTER), router_params);
        }
        efree(router_params);

    } else {
        group_router_base_name = emalloc(group_router_name_length);
        //普通路由
        snprintf(group_router_base_name, group_router_name_length, "#%s#", ZSTR_VAL(group_router_name));

        if (strstr(uri, ZSTR_VAL(group_router_name)) != NULL) {
            is_group_condition = 1;
            zend_update_static_property_string(lis_router_ce, ZEND_STRL(LIS_ROUTER_PROPERTY_NAME_PARAMS_STR_GROUP_ROUTER), "");
        }
    }


    zend_string_release(group_router_name);
    if (is_group_condition == 1) {
        //更新组变量
        zval res_group_router = {{0}};
        ZVAL_STRING(&res_group_router, group_router_base_name);
        zend_update_static_property(lis_router_ce, ZEND_STRL(LIS_ROUTER_PROPERTY_NAME_GROUP_ROUTE), &res_group_router);

        zval_ptr_dtor(&res_group_router);
        efree(group_router_base_name);

        return 0;

    }

    efree(group_router_base_name);
    return -1;
}
/* }}} */

/* {{{ proto LisRoute::get()
 */
PHP_METHOD(lis_router, get) 
{ 
    LIS_ROUTER_METHOD_QUERY(GET);
}
/* }}} */

/* {{{ proto LisRoute::post()
 */
PHP_METHOD(lis_router, post) 
{ 
    LIS_ROUTER_METHOD_QUERY(POST);
}
/* }}} */

/* {{{ proto LisRoute::put()
 */
PHP_METHOD(lis_router, put) 
{ 
    LIS_ROUTER_METHOD_QUERY(PUT);
}
/* }}} */

/* {{{ proto LisRoute::options()
 */
PHP_METHOD(lis_router, options) 
{ 
    LIS_ROUTER_METHOD_QUERY(OPTIONS);
}
/* }}} */

/* {{{ proto LisRoute::patch()
 */
PHP_METHOD(lis_router, patch) 
{ 
    LIS_ROUTER_METHOD_QUERY(PATCH);
}
/* }}} */

/* {{{ proto LisRoute::delete()
 */
PHP_METHOD(lis_router, delete) 
{ 
    LIS_ROUTER_METHOD_QUERY(DELETE);
}
/* }}} */

/* {{{ proto LisRoute::cli()
 */
PHP_METHOD(lis_router, cli) 
{ 
    LIS_ROUTER_METHOD_QUERY(Cli);
}
/* }}} */

/* {{{ proto LisRoute::middleware(string $middleware)
 */
PHP_METHOD(lis_router, middleware)
{
    if (LIS_G(hit_group_router) == 0) {
        RETURN_FALSE;
    }

    LIS_G(hit_group_router) = 0;

    smart_str buf = {0};
    zval iclass = {{0}};
    zend_class_entry *ce = NULL;
    char *class_lowercase, *action_lowercase;
    char *class, *entry, *action;

    if (LIS_G(hit_router) != 1) {
        RETURN_FALSE;
    }

    zend_string *middle_name;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "S", &middle_name) == FAILURE) {
        RETURN_FALSE;
    }

    char *name = ZSTR_VAL(middle_name);
    entry = estrndup(name, strlen(name));
    class = php_strtok_r(entry, ":", &action);

    class_lowercase = emalloc(strlen(class)+1);
    action_lowercase = emalloc(strlen(action)+1);

    zend_str_tolower_copy(class_lowercase, class, strlen(class));
    zend_str_tolower_copy(action_lowercase, action, strlen(action));

    smart_str_appendl(&buf, ZSTR_VAL(LIS_G(directory)), ZSTR_LEN(LIS_G(directory)));
    smart_str_appendc(&buf, DEFAULT_SLASH);
    smart_str_appendl(&buf, "library", strlen("library"));
    smart_str_appendc(&buf, DEFAULT_SLASH);
    smart_str_appendl(&buf, class, strlen(class));
    smart_str_appendc(&buf, '.');
    smart_str_appendl(&buf, LIS_EXT_NAME, LIS_EXT_LEN_NAME);
    smart_str_0(&buf);

    if (!lis_loader_import(buf.s, 0)) {
        lis_throw_exception(LIS_ERR_MIDDLEWARE_FAILED, "middleware class does not exist");
        goto out;
    } else if ((ce = zend_hash_str_find_ptr(EG(class_table), class_lowercase, strlen(class_lowercase))) == NULL) {
        lis_throw_exception(LIS_ERR_MIDDLEWARE_FAILED, "middleware class loading failed");
        goto out;
    }

    object_init_ex(&iclass, ce);

   if (zend_hash_str_exists(&(ce->function_table), ZEND_STRL("__construct"))) {
        zend_call_method_with_0_params(&iclass, ce, NULL, "__construct", NULL);
    }

    zend_call_method(&iclass, ce, NULL, action_lowercase, strlen(action_lowercase), NULL, 0, NULL, NULL);

    if (EG(exception)) {
        lis_throw_exception(LIS_ERR_MIDDLEWARE_FAILED, "middleware class exception");
        goto out;
    }

out:
    efree(entry);
    zval_ptr_dtor(&iclass);
    smart_str_free(&buf);
    efree(class_lowercase);
    efree(action_lowercase);
    return;
}
/* }}} */

/* {{{ proto LisRoute::group(string $name, mixed $func)
 */
PHP_METHOD(lis_router, group)
{
    if (LIS_G(hit_router) == 1) {
        lis_router_t *instance;
        instance = zend_read_static_property(lis_router_ce, ZEND_STRL(LIS_ROUTER_PROPERTY_NAME_INSTANCE), 1);
        RETURN_ZVAL(instance, 1, 0);
    }

    zval retval_ptr;
    zend_fcall_info fci;
    zend_string *name;
    zend_fcall_info_cache fci_cache;
 
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
            "Sf*", &name, &fci, &fci_cache, &fci.params, &fci.param_count) == FAILURE) {
        RETURN_FALSE;
    }
    char *uri = LIS_G(base_uri) ? ZSTR_VAL(LIS_G(base_uri)) : NULL;
    if (uri == NULL) {
        lis_set_group_router_null();
        RETURN_FALSE;
    }

    if (lis_check_group_router_params(name, uri) == 0) {

        fci.retval = &retval_ptr;

        if (zend_call_function(&fci, &fci_cache TSRMLS_CC) == FAILURE) {
            RETURN_FALSE;
        }

        LIS_G(hit_group_router) = 1;
    }

    //清理group正则
    lis_set_group_router_null();

    //处理中间件
    lis_router_t *ginstance;
    ginstance = zend_read_static_property(lis_router_ce, ZEND_STRL(LIS_ROUTER_PROPERTY_NAME_INSTANCE), 1);
    RETURN_ZVAL(ginstance, 1, 0);
}
/* }}} */

/* {{{ lis_router_methods
*/
zend_function_entry lis_router_methods[] = {
    PHP_ME(lis_router, group,       lis_router_group_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(lis_router, get,         lis_router_query_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(lis_router, post,        lis_router_query_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(lis_router, put,         lis_router_query_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(lis_router, cli,         lis_router_query_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(lis_router, patch,       lis_router_query_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(lis_router, delete,      lis_router_query_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(lis_router, options,     lis_router_query_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(lis_router, middleware,  lis_router_middleware_arginfo, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ LIS_STARTUP_FUNCTION
*/
LIS_STARTUP_FUNCTION(router) 
{

    zend_class_entry ce;

    LIS_INIT_CLASS_ENTRY(ce, "LisRoute", "Lis\\Route", lis_router_methods);
    lis_router_ce = zend_register_internal_class_ex(&ce, NULL);
    lis_router_ce->ce_flags |= ZEND_ACC_FINAL;

    zend_declare_property_null(lis_router_ce, ZEND_STRL(LIS_ROUTER_PROPERTY_NAME_INSTANCE),  ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
    zend_declare_property_null(lis_router_ce, ZEND_STRL(LIS_ROUTER_PROPERTY_NAME_GROUP_ROUTE),  ZEND_ACC_PUBLIC|ZEND_ACC_STATIC);

    zend_declare_property_null(lis_router_ce, ZEND_STRL(LIS_ROUTER_PROPERTY_NAME_ROUTER),  ZEND_ACC_PUBLIC|ZEND_ACC_STATIC);

    zend_declare_property_null(lis_router_ce, ZEND_STRL(LIS_ROUTER_PROPERTY_NAME_PARAMS_STR_GROUP_ROUTER),  ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);

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
