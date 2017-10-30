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
#include "Zend/zend_interfaces.h"
#include "ext/standard/url.h"
#include "zend_smart_str.h"
#include "main/SAPI.h"
#include "php_ini.h"

#include "php_lis.h"
#include "lis_request.h"
#include "lis_loader.h"
#include "lis_exception.h"
#include "lis_application.h"

zend_class_entry *lis_request_ce;

/* {{{ ARG_INFO
 */
ZEND_BEGIN_ARG_INFO_EX(lis_request_void_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_request_set_baseuri_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, base_uri)
ZEND_END_ARG_INFO()
/* }}} */

void lis_request_instance() /* {{{ */
{
    lis_request_t *instance;
    zval method;
    instance = zend_read_static_property(lis_request_ce, ZEND_STRL(LIS_REQUEST_PROPERTY_NAME_INSTANCE), 1);

    if (UNEXPECTED(IS_OBJECT == Z_TYPE_P(instance))) {
        return;
    }

    object_init_ex(instance, lis_request_ce);

    zend_update_static_property(lis_request_ce, ZEND_STRL(LIS_REQUEST_PROPERTY_NAME_INSTANCE), instance);

    if (SG(request_info).request_method) {
        ZVAL_STRING(&method, (char *)SG(request_info).request_method);
    } else if (strncasecmp(sapi_module.name, "cli", 3)) {
            ZVAL_STRING(&method, "Unknow");
    } else {
        ZVAL_STRING(&method, "Cli");
    }

    zend_update_static_property(lis_request_ce, ZEND_STRL(LIS_REQUEST_PROPERTY_NAME_METHOD), &method);
    zval_ptr_dtor(&method);

    //获取base_uri
    zval *uri;
    zend_string *settled_uri = NULL;
    do {
        uri = lis_request_query(TRACK_VARS_SERVER, 1, "PATH_INFO", sizeof("PATH_INFO") - 1);
        if (uri) {
            php_error_docref(NULL, E_WARNING, "Failed allocating %s bytes", Z_STRVAL_P(uri));
            if (EXPECTED(Z_TYPE_P(uri) == IS_STRING)) {
                settled_uri = zend_string_copy(Z_STR_P(uri));
                break;
            }
        }

        uri = lis_request_query(TRACK_VARS_SERVER, 1, "REQUEST_URI", sizeof("REQUEST_URI") - 1);
        if (uri) {
            if (EXPECTED(Z_TYPE_P(uri) == IS_STRING)) {
            if (strncasecmp(Z_STRVAL_P(uri), "http", sizeof("http") - 1) == 0) {
                php_url *url_info = php_url_parse(Z_STRVAL_P(uri));
                if (url_info && url_info->path) {
                        settled_uri = zend_string_init(url_info->path, strlen(url_info->path), 0);
                    }
                    php_url_free(url_info);
                } else {
                    char *pos = NULL;
                    if ((pos = strstr(Z_STRVAL_P(uri), "?"))) {
                        settled_uri = zend_string_init(Z_STRVAL_P(uri), pos - Z_STRVAL_P(uri), 0);
                    } else {
                        settled_uri = zend_string_copy(Z_STR_P(uri));
                    }
                }
                break;
            }
        }

        //nginx环境下，如果uri含有中文PATH_INFO会被截断，会产生一个ORIG_PATH_INFO
        uri = lis_request_query(TRACK_VARS_SERVER, 1, "ORIG_PATH_INFO", sizeof("ORIG_PATH_INFO") - 1);
        if (uri) {
            if (EXPECTED(Z_TYPE_P(uri) == IS_STRING)) {
                settled_uri = zend_string_copy(Z_STR_P(uri));
                break;
            }
        }
    } while (0);

    if (settled_uri) {
        char *p = ZSTR_VAL(settled_uri);

        //去掉/符号
        while (*p == '/' && *(p + 1) == '/') {
            p++;
        }

        if (p != ZSTR_VAL(settled_uri)) {
            zend_string *garbage = settled_uri;
            settled_uri = zend_string_init(p, ZSTR_LEN(settled_uri) - (p - ZSTR_VAL(settled_uri)), 0);
            zend_string_release(garbage);
        }

        LIS_G(base_uri) = zend_string_copy(settled_uri);

        zend_update_static_property_string(lis_request_ce, ZEND_STRL(LIS_REQUEST_PROPERTY_NAME_BASE_URI), ZSTR_VAL(settled_uri));
        zend_string_release(settled_uri);
    }

    return;
}
/* }}} */

zval *lis_request_get_method()  /* {{{ */
{
    zval *method;
    method = zend_read_static_property(lis_request_ce, ZEND_STRL(LIS_REQUEST_PROPERTY_NAME_METHOD), 1);
    return method;
}
/* }}} */

void lis_request_set_controller_action(char *controller_name, char *action_name) /* {{{ */
{
    zend_update_static_property_string(lis_request_ce, ZEND_STRL(LIS_REQUEST_PROPERTY_NAME_CONTROLLER_NAME), controller_name);
    zend_update_static_property_string(lis_request_ce, ZEND_STRL(LIS_REQUEST_PROPERTY_NAME_ACTION_NAME), action_name);
}
/* }}} */

void lis_request_load_controller(int is_init) /* {{{ */
{
    int status = 0;
    zval *controller;
    zval *action;
    zend_class_entry *ce = NULL;
    char *class_lowercase, *action_lowercase;
    smart_str buf = {0};
    zval icontroller = {{0}};

    controller = zend_read_static_property(lis_request_ce, ZEND_STRL(LIS_REQUEST_PROPERTY_NAME_CONTROLLER_NAME), 1);
    action = zend_read_static_property(lis_request_ce, ZEND_STRL(LIS_REQUEST_PROPERTY_NAME_ACTION_NAME), 1);
    class_lowercase = emalloc(strlen(Z_STRVAL_P(controller))+1);
    action_lowercase = emalloc(strlen(Z_STRVAL_P(action))+1);

    zend_str_tolower_copy(class_lowercase, Z_STRVAL_P(controller), Z_STRLEN_P(controller));
    zend_str_tolower_copy(action_lowercase, Z_STRVAL_P(action), Z_STRLEN_P(action));

    smart_str_appendl(&buf, ZSTR_VAL(LIS_G(directory)), ZSTR_LEN(LIS_G(directory)));
    smart_str_appendc(&buf, DEFAULT_SLASH);
    smart_str_appendl(&buf, "controller", strlen("controller"));
    smart_str_appendc(&buf, DEFAULT_SLASH);
    smart_str_appendl(&buf, Z_STRVAL_P(controller), Z_STRLEN_P(controller));
    smart_str_appendc(&buf, '.');
    smart_str_appendl(&buf, LIS_EXT_NAME, LIS_EXT_LEN_NAME);
    smart_str_0(&buf);

    if (!lis_loader_import(buf.s, 0)) {
        lis_throw_exception(LIS_ERR_CONTROLLER_FAILED, "Controller class does not exist");
        goto out;
    } else if ((ce = zend_hash_str_find_ptr(EG(class_table), class_lowercase, strlen(class_lowercase))) == NULL) {
        lis_throw_exception(LIS_ERR_CONTROLLER_FAILED, "Controller class loading failed");
        goto out;
    }

    smart_str_free(&buf);

    if (is_init == 1) {
        //init controller class
        object_init_ex(&icontroller, ce);

        if (zend_hash_str_exists(&(ce->function_table), ZEND_STRL("__construct"))) {
            zend_call_method_with_0_params(&icontroller, ce, NULL, "__construct", NULL);
        }

        if (zend_hash_str_exists(&(ce->function_table), action_lowercase, strlen(action_lowercase))) {
            LIS_G(hit_action) = 1;
            zend_call_method(&icontroller, ce, NULL, action_lowercase, strlen(action_lowercase), NULL, 0, NULL, NULL);
        } else {
            lis_throw_exception(LIS_ERR_CONTROLLER_FAILED, "Controller class action not exist");
            goto out;
        }
    }

    if (EG(exception)) {
        lis_throw_exception(LIS_ERR_CONTROLLER_FAILED, "Controller class exception");
        goto out;
    }


out:
    zval_ptr_dtor(&icontroller);
    smart_str_free(&buf);
    efree(class_lowercase);
    efree(action_lowercase);
    return;
}
/* }}} */

static zval *lis_request_query(uint type, zend_bool str_type, void *name, size_t len) /* {{{ */
{
    zval *carrier = NULL, *ret; 

    zend_bool jit_init = PG(auto_globals_jit);

    switch (type) {
        case TRACK_VARS_POST:
        case TRACK_VARS_GET:
        case TRACK_VARS_FILES:
        case TRACK_VARS_COOKIE:
            carrier = &PG(http_globals)[type];
            break;
        case TRACK_VARS_ENV:
            if (jit_init) {
                zend_string *env_str = zend_string_init("_ENV", sizeof("_ENV") - 1, 0);
                zend_is_auto_global(env_str);
                zend_string_release(env_str);
            }
            carrier = &PG(http_globals)[type];
            break;
        case TRACK_VARS_SERVER:
            if (jit_init) {
                zend_string *server_str = zend_string_init("_SERVER", sizeof("_SERVER") - 1, 0);
                zend_is_auto_global(server_str);
                zend_string_release(server_str);
            }
            carrier = &PG(http_globals)[type];
            break;
        case TRACK_VARS_REQUEST:
            if (jit_init) {
                zend_string *request_str = zend_string_init("_REQUEST", sizeof("_REQUEST") - 1, 0);
                zend_is_auto_global(request_str);
                zend_string_release(request_str);
            }
            carrier = zend_hash_str_find(&EG(symbol_table), ZEND_STRL("_REQUEST")); 
            break;
        default:
            break;
    }

    if (!carrier) {
        return NULL;
    }

    if (!name) {
        return carrier;
    }

    if (EXPECTED(str_type)) {
        zend_string *zname;
        zname = zend_string_init((char *)name, strlen(name), 0);
        if ((ret = zend_hash_find(Z_ARRVAL_P(carrier), zname)) == NULL) {
            zend_string_release(zname);
            return NULL;
        }
        zend_string_release(zname);
    } else {
        if ((ret = zend_hash_str_find(Z_ARRVAL_P(carrier), (char *)name, len)) == NULL) {
            return NULL;
        }
    }
    return ret;
}
/* }}} */

/** proto public LisRequest::getQuery(mixed $name, mixed $default = NULL)
 */
LIS_REQUEST_METHOD(lis_request, Query, TRACK_VARS_GET);

/** proto public LisRequest::getPost(mixed $name, mixed $default = NULL)
 */
LIS_REQUEST_METHOD(lis_request, Post, TRACK_VARS_POST);

/** proto public LisRequest::getCookie(mixed $name, mixed $default = NULL)
 */
LIS_REQUEST_METHOD(lis_request, Cookie, TRACK_VARS_COOKIE);

/** proto public LisRequest::getFiles(mixed $name, mixed $default = NULL)
 */
LIS_REQUEST_METHOD(lis_request, Files, TRACK_VARS_FILES);

/** proto public LisRequest::getRequest(mixed $name, mixed $default = NULL)
 */
LIS_REQUEST_METHOD(lis_request, Request, TRACK_VARS_REQUEST);

/** proto public LisRequest::getControllerName()
 */
LIS_REQUEST_PARAMS_METHOD(lis_request, ControllerName, LIS_REQUEST_PROPERTY_NAME_CONTROLLER_NAME);

/** proto public LisRequest::getActionName()
 */
LIS_REQUEST_PARAMS_METHOD(lis_request, ActionName, LIS_REQUEST_PROPERTY_NAME_ACTION_NAME);

/** proto public LisRequest::getMethod()
 */
LIS_REQUEST_PARAMS_METHOD(lis_request, Method, LIS_REQUEST_PROPERTY_NAME_METHOD);

/** proto public LisRequest::getBaseUri()
 */
LIS_REQUEST_PARAMS_METHOD(lis_request, BaseUri, LIS_REQUEST_PROPERTY_NAME_BASE_URI);

/* {{{  proto public LisRequest::setBaseUri(string $base_uri)
 */
PHP_METHOD(lis_request, setBaseUri)
{
    zend_string *base_uri;
    int base_uri_length;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "S", &base_uri, &base_uri_length) == FAILURE) {
        RETURN_FALSE;
    }

    LIS_G(base_uri) = zend_string_copy(base_uri);

    zend_update_static_property_string(lis_request_ce, ZEND_STRL(LIS_REQUEST_PROPERTY_NAME_BASE_URI), ZSTR_VAL(base_uri));
    RETURN_TRUE;
}
/* }}} */

/* {{{ lis_request_methods
 */
zend_function_entry lis_request_methods[] = {
    PHP_ME(lis_request, getQuery,           NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(lis_request, getPost,            NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(lis_request, getFiles,           NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(lis_request, getCookie,          NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(lis_request, getRequest,         NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(lis_request, getControllerName,  NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(lis_request, getActionName,      NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(lis_request, getMethod,          NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(lis_request, getBaseUri,         NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(lis_request, setBaseUri,         lis_request_set_baseuri_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ LIS_STARTUP_FUNCTION
 */
LIS_STARTUP_FUNCTION(request) 
{
    zend_class_entry ce;

    LIS_INIT_CLASS_ENTRY(ce, "LisRequest", "Lis\\Request", lis_request_methods);
    lis_request_ce = zend_register_internal_class_ex(&ce, NULL);
    lis_request_ce->ce_flags |= ZEND_ACC_FINAL;

    zend_declare_property_null(lis_request_ce, ZEND_STRL(LIS_REQUEST_PROPERTY_NAME_INSTANCE),  ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
    zend_declare_property_null(lis_request_ce, ZEND_STRL(LIS_REQUEST_PROPERTY_NAME_CONTROLLER_NAME),  ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
    zend_declare_property_null(lis_request_ce, ZEND_STRL(LIS_REQUEST_PROPERTY_NAME_ACTION_NAME),  ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
    zend_declare_property_null(lis_request_ce, ZEND_STRL(LIS_REQUEST_PROPERTY_NAME_METHOD),  ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
    zend_declare_property_null(lis_request_ce, ZEND_STRL(LIS_REQUEST_PROPERTY_NAME_BASE_URI),  ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);

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
