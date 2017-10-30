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
#include "lis_exception.h"
#include "lis_view.h"
#include "lis_application.h"

zend_class_entry *lis_view_ce;

/* {{{ ARG_INFO
 */
ZEND_BEGIN_ARG_INFO_EX(lis_view_void_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_view_assign_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, name)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_view_render_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, tpl)
    ZEND_ARG_INFO(0, vars)
ZEND_END_ARG_INFO()

/* }}} */

void lis_view_instance() /* {{{ */
{
    lis_view_t *instance;
    instance = zend_read_static_property(lis_view_ce, ZEND_STRL(LIS_VIEW_PROPERTY_NAME_INSTANCE), 1);

    if (UNEXPECTED(IS_OBJECT == Z_TYPE_P(instance))) {
        return;
    }

    object_init_ex(instance, lis_view_ce);

    zend_update_static_property(lis_view_ce, ZEND_STRL(LIS_VIEW_PROPERTY_NAME_INSTANCE), instance);

    zval tpl_vars;
    array_init(&tpl_vars);
    zend_update_static_property(lis_view_ce, ZEND_STRL(LIS_VIEW_PROPERTY_NAME_TPLVARS), &tpl_vars);

    zval_ptr_dtor(&tpl_vars);
    return;
}
/* }}} */

static int lis_view_valid_var_name(char *var_name, int len) /* {{{ 变量名验证 */
{
    int i, ch; 

    if (!var_name)
        return 0;

    ch = (int)((unsigned char *)var_name)[0];

    if (ch >= 48 && ch <= 39) {
        return 0;
    }

    if (len > 1) {
        for (i = 1; i < len; i++) {
            ch = (int)((unsigned char *)var_name)[i];
            if ((ch < 48 || ch > 57)  &&  
                (ch < 65 || ch > 90)  &&  
                (ch < 97 || ch > 122) &&
                (ch != 95)
            ) {
                return 0;
            }
        }
    }
    
    return 1;
}
/* }}} */

static int lis_view_render(lis_view_t *view, zend_string *tpl, zval *vars, zval *return_val) /* {{{ */
{
    zval *tpl_vars;
    zval *entry;
    zend_string *var_name;
    zend_array *symbol_table;
    if (ZSTR_VAL(tpl) == NULL) {
        return 0;
    }

    tpl_vars = zend_read_static_property(lis_view_ce, ZEND_STRL(LIS_VIEW_PROPERTY_NAME_TPLVARS), 1);

    zend_class_entry *scope = zend_get_executed_scope();
    symbol_table = emalloc(sizeof(zend_array));
    zend_hash_init(symbol_table, 8, NULL, ZVAL_PTR_DTOR, 0);
    zend_hash_real_init(symbol_table , 0);

    if (tpl_vars && Z_TYPE_P(tpl_vars) == IS_ARRAY) {

        ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(tpl_vars), var_name, entry) {

            if (zend_string_equals_literal(var_name, "GLOBALS")) { 
                continue;
            }

            if (zend_string_equals_literal(var_name, "this")  && scope && ZSTR_LEN(scope->name) != 0) {
                continue;
            }

            if (lis_view_valid_var_name(ZSTR_VAL(var_name), ZSTR_LEN(var_name))) { 
                if (EXPECTED(zend_hash_add_new(symbol_table, var_name, entry))) {  
                    Z_TRY_ADDREF_P(entry);
                }
            }

        } ZEND_HASH_FOREACH_END();
    }


    if (vars && Z_TYPE_P(vars) == IS_ARRAY) {

        ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(vars), var_name, entry) {

            if (zend_string_equals_literal(var_name, "GLOBALS")) { 
                continue;
            }

            if (zend_string_equals_literal(var_name, "this")  && scope && ZSTR_LEN(scope->name) != 0) {
                continue;
            }

            if (lis_view_valid_var_name(ZSTR_VAL(var_name), ZSTR_LEN(var_name))) { 
                if (EXPECTED(zend_hash_add_new(symbol_table, var_name, entry))) {  
                    Z_TRY_ADDREF_P(entry);
                }
            }

        } ZEND_HASH_FOREACH_END();
    }

    zend_string *script;
    script = strpprintf(0, "%s%cview%c%s.php", ZSTR_VAL(LIS_G(directory)), DEFAULT_SLASH, DEFAULT_SLASH, ZSTR_VAL(tpl)); 

    if (lis_view_render_tpl(view, symbol_table, script, return_val) == 0) {
        zend_array_destroy(symbol_table);
        zend_string_release(script); 
        return 0;
    }

    zend_string_release(script); 
    zend_array_destroy(symbol_table);
    return 1;
}
/* }}} */

static int lis_view_exec_tpl(lis_view_t *view, zend_op_array *op_array, zend_array *symbol_table, zval* ret) /* {{{ */
{
    zend_execute_data *call;
    zval result;

    ZVAL_UNDEF(&result);

    op_array->scope = Z_OBJCE_P(view);

    //从堆栈中分配一块内存给上下文
    call = zend_vm_stack_push_call_frame(ZEND_CALL_NESTED_CODE
#if PHP_VERSION_ID >= 70100
            | ZEND_CALL_HAS_SYMBOL_TABLE
#endif
            ,
            (zend_function*)op_array, 0, op_array->scope, Z_OBJ_P(view));

    call->symbol_table = symbol_table;

    if (ret && php_output_start_user(NULL, 0, PHP_OUTPUT_HANDLER_STDFLAGS) == FAILURE) {
        php_error_docref("ref.outcontrol", E_WARNING, "failed to create buffer");
        return 0;
    }

    zend_init_execute_data(call, op_array, &result);

    ZEND_ADD_CALL_FLAG(call, ZEND_CALL_TOP);
    zend_execute_ex(call);
    zend_vm_stack_free_call_frame(call);

    zval_ptr_dtor(&result);

    if (UNEXPECTED(EG(exception) != NULL)) {
        if (ret) {
            php_output_discard();
        }
        return 0;
    }

    if (ret) {
        if (php_output_get_contents(ret) == FAILURE) {
            php_output_end();
            php_error_docref(NULL, E_WARNING, "Unable to fetch ob content");
            return 0;
        }

        if (php_output_discard() != SUCCESS ) {
            return 0;
        }
    }

    return 1;
}
/* }}} */

static int lis_view_render_tpl(lis_view_t *view, zend_array *symbol_table, zend_string *tpl, zval *ret) /* {{{ */
{
    //加载php模板文件
    int status = 0;
    zend_file_handle file_handle;
    zend_op_array   *op_array;
    char realpath[MAXPATHLEN];

    if (!VCWD_REALPATH(ZSTR_VAL(tpl), realpath)) {
        lis_throw_exception(LIS_ERR_VIEW_FAILED, "Failed opening template %s: %s", ZSTR_VAL(tpl), strerror(errno));
        return 0;
    }   

    file_handle.filename = ZSTR_VAL(tpl);
    file_handle.free_filename = 0;
    file_handle.type = ZEND_HANDLE_FILENAME;
    file_handle.opened_path = NULL;
    file_handle.handle.fp = NULL;

    op_array = zend_compile_file(&file_handle, ZEND_INCLUDE);

    if (op_array) {
        if (file_handle.handle.stream.handle) {
            if (!file_handle.opened_path) {
                file_handle.opened_path = zend_string_copy(tpl);
            }   
            zend_hash_add_empty_element(&EG(included_files), file_handle.opened_path);
        }   

        status = lis_view_exec_tpl(view, op_array, symbol_table, ret);

        destroy_op_array(op_array);
        efree_size(op_array, sizeof(zend_op_array));
    }   

    zend_destroy_file_handle(&file_handle);

    return status;
}
/* }}} */

PHP_METHOD(lis_view, assign) /* {{{ */
{
    zend_string *name;
    zval *value;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "Sz", &name, &value) == FAILURE) {
        RETURN_FALSE;
    }

    zval *tpl_vars = zend_read_static_property(lis_view_ce, ZEND_STRL(LIS_VIEW_PROPERTY_NAME_TPLVARS), 1);

    if (zend_hash_update(Z_ARRVAL_P(tpl_vars), name , value) != NULL) {
        Z_TRY_ADDREF_P(value);
        RETURN_TRUE;
    }
    RETURN_FALSE;
}
/* }}} */

PHP_METHOD(lis_view, render) /* {{{ */
{
    zend_string *tpl;
    zval *vars;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "S|z", &tpl, &vars) == FAILURE) {
        RETURN_FALSE;
    }

    lis_view_t *instance;
    instance = zend_read_static_property(lis_view_ce, ZEND_STRL(LIS_VIEW_PROPERTY_NAME_INSTANCE), 1);
    if (!lis_view_render(instance, tpl, vars, NULL)) {
        RETURN_FALSE;
    }
}
/* }}} */

/* {{{ lis_view_methods
*/
zend_function_entry lis_view_methods[] = {
    PHP_ME(lis_view, assign, lis_view_assign_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(lis_view, render, lis_view_render_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ LIS_STARTUP_FUNCTION
*/
LIS_STARTUP_FUNCTION(view) {
    zend_class_entry ce;

    LIS_INIT_CLASS_ENTRY(ce, "LisView", "Lis\\View", lis_view_methods);
    lis_view_ce = zend_register_internal_class_ex(&ce, NULL);
    lis_view_ce->ce_flags |= ZEND_ACC_FINAL;

    zend_declare_property_null(lis_view_ce, ZEND_STRL(LIS_VIEW_PROPERTY_NAME_INSTANCE),  ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
    zend_declare_property_null(lis_view_ce, ZEND_STRL(LIS_VIEW_PROPERTY_NAME_TPLVARS),  ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);

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


