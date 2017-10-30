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
#include "zend_smart_str.h"

#include "php_lis.h"
#include "lis_loader.h"
#include "lis_exception.h"
#include "lis_application.h"

zend_class_entry *lis_loader_ce;

/* {{{ ARG_INFO
 */
ZEND_BEGIN_ARG_INFO_EX(lis_loader_void_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_loader_autoloader_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, class_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_loader_import_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, file_name)
ZEND_END_ARG_INFO()
/* }}} */

void lis_loader_instance() /* {{{ */
{
    lis_loader_t *instance;
    instance = zend_read_static_property(lis_loader_ce, ZEND_STRL(LIS_LOADER_PROPERTY_NAME_INSTANCE), 1);

    if (UNEXPECTED(IS_OBJECT == Z_TYPE_P(instance))) {
        return;
    }

    object_init_ex(instance, lis_loader_ce);

    zval rinstance;
    ZVAL_COPY(&rinstance, instance);
    //注册spl 自动注册机 autoload函数 自动加载类 手动加载类函数,支持目录
    if (!lis_loader_register(&rinstance)) {
        php_error_docref(NULL, E_WARNING, "Failed to register autoload function");
        return;
    }

    zend_update_static_property(lis_loader_ce, ZEND_STRL(LIS_LOADER_PROPERTY_NAME_INSTANCE), instance);

    return;
}
/* }}} */

static int lis_loader_register(lis_loader_t *loader) /* {{{ */
{
    zval autoload, func, method, ret;

    array_init(&autoload);
    ZVAL_STRING(&method, LIS_AUTOLOAD_FUNC_NAME);

    zend_hash_next_index_insert(Z_ARRVAL_P(&autoload), loader);
    zend_hash_next_index_insert(Z_ARRVAL_P(&autoload), &method);

    ZVAL_STRING(&func, LIS_SPL_AUTOLOAD_REGISTER_NAME);

    zend_fcall_info fci = {
        sizeof(fci), 
#if PHP_VERSION_ID < 70100
        EG(function_table),
#endif
        func,
#if PHP_VERSION_ID < 70100
        NULL,
#endif
        &ret,
        &autoload,
        NULL,
        1,
        1
    };

    if (zend_call_function(&fci, NULL) == FAILURE) {
        zval_ptr_dtor(&func);
        zval_ptr_dtor(&autoload);
        php_error_docref(NULL, 
            E_WARNING, 
            "Unable to register autoload function %s",
            LIS_AUTOLOAD_FUNC_NAME);
        return 0;
    }

    zval_ptr_dtor(&func);
    zval_ptr_dtor(&autoload);

    return 1;
}
/* }}} */

int lis_loader_import(zend_string *path, int use_path) /* {{{ */
{
    zend_file_handle file_handle;
    zend_op_array *op_array;
    char realpath[MAXPATHLEN];

    if (!VCWD_REALPATH(ZSTR_VAL(path), realpath)) {
        return 0;
    }

    file_handle.filename = ZSTR_VAL(path);
    file_handle.free_filename = 0;
    file_handle.type = ZEND_HANDLE_FILENAME;
    file_handle.opened_path = NULL;
    file_handle.handle.fp = NULL;

    op_array = zend_compile_file(&file_handle, ZEND_INCLUDE);

    if (op_array && file_handle.handle.stream.handle) {
        if (!file_handle.opened_path) {
            file_handle.opened_path = zend_string_copty(path);
        }

        zend_hash_add_empty_element(&EG(included_files), file_handle.opened_path);
    }

    zend_destroy_file_handle(&file_handle);
    zend_string_release(path);

    if (op_array) {
        zval result;

        ZVAL_UNDEF(&result);
        zend_execute(op_array, &result);

        destroy_op_array(op_array);
        efree(op_array);
        if (!EG(exception)) {
            zval_ptr_dtor(&result);
        }

        return 1;
    }

    return 0;
} 
/* }}} */

static int lis_loader_autoload(char *file_name, size_t name_length, char **directory) /* {{{ */
{
    int status;
    size_t directory_length;
    smart_str buf = {0};

    smart_str_appendl(&buf, *directory, strlen(*directory));
    efree(*directory);

    directory_length = ZSTR_LEN(buf.s);

    smart_str_appendc(&buf, DEFAULT_SLASH);
    smart_str_appendl(&buf, file_name, name_length);
    smart_str_appendc(&buf, '.');
    smart_str_appendl(&buf, LIS_EXT_NAME, LIS_EXT_LEN_NAME);

    smart_str_0(&buf);

    if (directory) {
        *(directory) = estrndup(ZSTR_VAL(buf.s), ZSTR_LEN(buf.s));
    }

    status = lis_loader_import(buf.s, 0);
    smart_str_free(&buf);
    return status;
}
/* }}} */

/* {{{ proto LisLoader::import(string $file_name)
 */
PHP_METHOD(lis_loader, import)
{
    zend_string *file_name;
    int file_name_length;
    int retval = 0;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "S", &file_name, &file_name_length) == FAILURE) {
        return;
    }

    retval = zend_hash_exists(&EG(included_files), file_name);
    if (retval) {
        RETURN_TRUE;
    }
    retval = lis_loader_import(file_name, 0);
    RETURN_BOOL(retval);
}
/* }}} */

/* {{{ proto LisLoader::autoload(string $class_name)
 */
PHP_METHOD(lis_loader, autoload) /* {{{ */
{
    char *class_name, *file_name = NULL, *app_directory, *directory = NULL;
    size_t file_name_length = 0;
    size_t class_name_length;
    zend_bool ret = 0x01;
 
    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "s", &class_name, &class_name_length) == FAILURE) {
        RETURN_FALSE;
    }

    app_directory = LIS_G(directory) ? ZSTR_VAL(LIS_G(directory)) : NULL; 

    //如果类名没有controller 和 model ，则直接去library目录查找
    if ((strstr(class_name, "Lis") != NULL)) {
        spprintf(&directory, 0, "%s%c%s", app_directory, DEFAULT_SLASH, LIS_DIRECTORY_NAME_LIBRARY);
        file_name_length = class_name_length;
        file_name = estrndup(class_name, file_name_length);
    } else {
        if(strncmp(class_name+(class_name_length-strlen(LIS_LOADER_MODEL)), LIS_LOADER_MODEL, LIS_LOADER_LEN_MODEL) == 0) {
            spprintf(&directory, 0, "%s%c%s", app_directory, DEFAULT_SLASH, LIS_DIRECTORY_NAME_MODEL); 
            file_name_length = class_name_length; 
            file_name = estrndup(class_name, file_name_length); 
        } else { 
            spprintf(&directory, 0, "%s%c%s", app_directory, DEFAULT_SLASH, LIS_DIRECTORY_NAME_PLUGIN);
            file_name_length = class_name_length; 
            file_name = estrndup(class_name, file_name_length); 
        }
    }

    if (file_name == NULL) {
        php_error_docref(NULL,
            E_WARNING,
            "Couldn't load a MVC class unless an %s is initialized",
            ZSTR_VAL(lis_application_ce->name));
        ret = 0;
        goto out;
    }

    char *lower_case_name = zend_str_tolower_dup(class_name, class_name_length);
    if (lis_loader_autoload(file_name, file_name_length, &directory) &&
        zend_hash_str_exists(EG(class_table), lower_case_name, class_name_length)) {
        efree(lower_case_name);
        goto out;
    }

    efree(lower_case_name);
    ret = 0;
    goto out;

out:
    if (directory) {
        efree(directory);
    }
    if (file_name) {
        efree(file_name);
    }
    RETURN_BOOL(ret);
}
/* }}} */

/* {{{ lis_loader_methods
*/
zend_function_entry lis_loader_methods[] = {
    PHP_ME(lis_loader, autoload, lis_loader_autoloader_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(lis_loader, import, lis_loader_import_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ LIS_STARTUP_FUNCTION
*/
LIS_STARTUP_FUNCTION(loader) 
{
    zend_class_entry ce;

    LIS_INIT_CLASS_ENTRY(ce, "LisLoader", "Lis\\Loader", lis_loader_methods);
    lis_loader_ce = zend_register_internal_class_ex(&ce, NULL);
    lis_loader_ce->ce_flags |= ZEND_ACC_FINAL;

    zend_declare_property_null(lis_loader_ce, ZEND_STRL(LIS_LOADER_PROPERTY_NAME_INSTANCE),  ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);

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
