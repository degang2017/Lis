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
#include "zend_smart_str.h"
#include "php_ini.h"

#include "php_lis.h"
#include "lis_model.h"
#include "lis_exception.h"
#include "lis_application.h"

zend_class_entry *lis_model_ce;

/* {{{ ARG_INFO
 */
ZEND_BEGIN_ARG_INFO_EX(lis_model_void_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_model_table_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, table_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_model_columns_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, columns)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_model_where_arginfo, 0, 0, 1)
    ZEND_ARG_ARRAY_INFO(0, where, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_model_group_arginfo, 0, 0, 1)
    ZEND_ARG_ARRAY_INFO(0, group, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_model_order_arginfo, 0, 0, 1)
    ZEND_ARG_ARRAY_INFO(0, order, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_model_prepare_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, prepare)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_model_join_arginfo, 0, 0, 1)
    ZEND_ARG_ARRAY_INFO(0, left_join, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_model_insert_arginfo, 0, 0, 1)
    ZEND_ARG_ARRAY_INFO(0, insert, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_model_update_arginfo, 0, 0, 2)
    ZEND_ARG_ARRAY_INFO(0, set, 1)
    ZEND_ARG_ARRAY_INFO(0, where, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(lis_model_delete_arginfo, 0, 0, 1)
    ZEND_ARG_ARRAY_INFO(0, where, 1)
ZEND_END_ARG_INFO()

/* }}} */

static void lis_model_where_analysis(zend_string *key, zval *val, smart_str *buf, int is_first) /* {{{ */
{

    char *start_position = strchr(ZSTR_VAL(key), '[');

    if (start_position != NULL) {

        char *field, *division, *op_type;
        char *k_value = ZSTR_VAL(key);
        zval *in_val, *between;
        uint32_t in_length = 0;
        ulong idx = 0;
        ulong between_idx = 0;

        division = estrndup(k_value, strlen(k_value));
        field = php_strtok_r(division, "[", &op_type);

        *(op_type+(strlen(op_type)-1)) = '\0';
        if (strcmp(field, "OR") == 0 || strcmp(field, "AND") == 0) {
            efree(division);
            return;
        }

        if (is_first > 1) {
            smart_str_appendl(buf, " AND", 4);
        }

        if (strcmp(op_type, "=") == 0 
            || strcmp(op_type, "!=") == 0 
            || strcmp(op_type, ">") == 0 
            || strcmp(op_type, ">=") == 0 
            || strcmp(op_type, "<") == 0 
            || strcmp(op_type, "<=") == 0 ) {
            smart_str_appendl(buf, " ", 1);
            smart_str_appendl(buf, field, strlen(field));

            smart_str_appendl(buf, op_type, strlen(op_type));

            if (IS_STRING == Z_TYPE_P(val)) {
                smart_str_appendl(buf, "'", 1);
                smart_str_appendl(buf, Z_STRVAL_P(val), Z_STRLEN_P(val));
                smart_str_appendl(buf, "'", 1);
            } else {
                convert_to_string_ex(val);
                smart_str_appendl(buf, Z_STRVAL_P(val), Z_STRLEN_P(val));
            }
        } else if (strcmp(op_type, "%") == 0) {
            smart_str_appendl(buf, " ", 1);
            smart_str_appendl(buf, field, strlen(field));
            smart_str_appendl(buf, " LIKE '", 7);
            smart_str_appendl(buf, Z_STRVAL_P(val), Z_STRLEN_P(val));
            smart_str_appendl(buf, "' ", 1);
        } else if (strcmp(op_type, "!") == 0 || strcmp(op_type, "~!") == 0 ) {
            smart_str_appendl(buf, " ", 1);
            smart_str_appendl(buf, field, strlen(field));

            in_length = zend_array_count(Z_ARRVAL_P(val));

            if (strcmp(op_type, "~!") == 0 ) {
                smart_str_appendl(buf, " NOT IN (", 9);
            } else {
                smart_str_appendl(buf, " IN (", 5);
            }
            ZEND_HASH_FOREACH_NUM_KEY_VAL(Z_ARRVAL_P(val), idx, in_val){
                if (IS_STRING == Z_TYPE_P(in_val)) {
                    smart_str_appendl(buf, "'", 1);
                    smart_str_appendl(buf, Z_STRVAL_P(in_val), Z_STRLEN_P(in_val));
                    smart_str_appendl(buf, "'", 1);
                } else {
                    convert_to_string_ex(in_val);
                    smart_str_appendl(buf, Z_STRVAL_P(in_val), Z_STRLEN_P(in_val));
                }

                if (idx < (in_length-1)) {
                    smart_str_appendl(buf, ",", 1);
                }
            } ZEND_HASH_FOREACH_END();

            smart_str_appendl(buf, ")", 1);
        } else if (strcmp(op_type, "<>") == 0 || strcmp(op_type, "><") == 0) {

            smart_str_appendl(buf, " ", 1);
            smart_str_appendl(buf, field, strlen(field));

            if (strcmp(op_type, "<>") == 0 ) {
                smart_str_appendl(buf, " BETWEEN ", 9);
            } else {
                smart_str_appendl(buf, " NOT BETWEEN ", 13);
            }

            ZEND_HASH_FOREACH_NUM_KEY_VAL(Z_ARRVAL_P(val), between_idx, between){
                if (IS_STRING == Z_TYPE_P(between)) {
                    smart_str_appendl(buf, "'", 1);
                    smart_str_appendl(buf, Z_STRVAL_P(between), Z_STRLEN_P(between));
                    smart_str_appendl(buf, "'", 1);
                } else {
                    convert_to_string_ex(between);
                    smart_str_appendl(buf, Z_STRVAL_P(between), Z_STRLEN_P(between));
                }

                if (between_idx == 0) {
                    smart_str_appendl(buf, " AND ", 5);
                }
            } ZEND_HASH_FOREACH_END();

        }

        efree(division);

    } else {
        if (is_first > 1) {
            smart_str_appendl(buf, " AND", 4);
        }

        smart_str_appendl(buf, " ", 1);
        smart_str_appendl(buf, ZSTR_VAL(key), ZSTR_LEN(key));
        smart_str_appendl(buf, "=", 1);

        if (IS_STRING == Z_TYPE_P(val)) {
            smart_str_appendl(buf, "'", 1);
            smart_str_appendl(buf, Z_STRVAL_P(val), Z_STRLEN_P(val));
            smart_str_appendl(buf, "'", 1);
        } else {
            convert_to_string_ex(val);
            smart_str_appendl(buf, Z_STRVAL_P(val), Z_STRLEN_P(val));
        }
    }    
}
/* }}} */

static void lis_model_and_logic(zend_string *key, zval *val, smart_str *buf, int is_first) /* {{{ */
{
    
    zend_string *a_key;
    zval *a_val;
    int and_array_count = 0;

    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(val), a_key, a_val){
        is_first++;
        lis_model_where_analysis(a_key, a_val, buf, is_first);
        if ((strcmp(ZSTR_VAL(a_key), "[OR]")) == 0) {
            smart_str_appendl(buf, " OR", 3);
            lis_model_and_logic(a_key, a_val, buf, 0);
        } else if ((strcmp(ZSTR_VAL(a_key), "[AND]")) == 0) {
            smart_str_appendl(buf, " AND", 4);
            lis_model_and_logic(a_key, a_val, buf, 0);
        }
    } ZEND_HASH_FOREACH_END();
}
/* }}} */

static void lis_model_join(zval *join, char *join_type, zval *obj) /* {{{ */
{

    zval *sql;
    smart_str buf = {0};

    sql = zend_read_property(lis_model_ce, obj, ZEND_STRL(LIS_MODEL_PROPERTY_NAME_SQL), 1, NULL);
    smart_str_appendl(&buf, Z_STRVAL_P(sql), Z_STRLEN_P(sql));
    smart_str_appendl(&buf, join_type, strlen(join_type));

    zval *val, *v_val;
    zend_string *key, *v_key;
       
    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(join), key, val){
        smart_str_appendl(&buf, ZSTR_VAL(key), ZSTR_LEN(key)); 
        smart_str_appendl(&buf, " on ", 4);

        ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(val), v_key, v_val){
            smart_str_appendl(&buf, ZSTR_VAL(v_key), ZSTR_LEN(v_key));
            smart_str_appendl(&buf, "=", 1);
            smart_str_appendl(&buf, Z_STRVAL_P(v_val), Z_STRLEN_P(v_val));
            smart_str_appendl(&buf, " ", 1);
        } ZEND_HASH_FOREACH_END();

    } ZEND_HASH_FOREACH_END();

    smart_str_appendl(&buf, " ", 1);
    smart_str_0(&buf);
    zend_update_property_string(lis_model_ce, obj, ZEND_STRL(LIS_MODEL_PROPERTY_NAME_SQL), ZSTR_VAL(buf.s));
    smart_str_free(&buf);
}
/* }}} */

static void lis_model_query(zval *obj, zval *ret) /* {{{ */
{

    zval *sql,*pdo;

    pdo = zend_read_static_property(lis_model_ce, ZEND_STRL(LIS_MODEL_PROPERTY_NAME_PDO), 1);
    sql = zend_read_property(lis_model_ce, obj, ZEND_STRL(LIS_MODEL_PROPERTY_NAME_SQL), 1, NULL);

    if (pdo != NULL) {
        zval func;

        zval params[1];
        ZVAL_STRING(&params[0], Z_STRVAL_P(sql));

        ZVAL_STRING(&func, "query");

        zend_fcall_info fci = {
            sizeof(fci), 
            func,
            ret,
            params,
            Z_OBJ_P(pdo),
            1,
            1
        };

        if (zend_call_function(&fci, NULL) == FAILURE) {
            php_error_docref(NULL, E_ERROR, "Pdo query failed");
            return;
        }

        zval_ptr_dtor(&func);
        zval_ptr_dtor(params);

    }
    return;
}
/* }}} */

/* {{{ proto LisModel::pdo()
 */
PHP_METHOD(lis_model, pdo) 
{
    zval *pdo;
    if ((pdo = zend_read_static_property(lis_model_ce, ZEND_STRL(LIS_MODEL_PROPERTY_NAME_PDO), 1)) == NULL) {
        RETURN_NULL();
    }
    RETURN_ZVAL(pdo, 1, 0);
}
/* }}} */

/* {{{ proto LisModel::where(array $where)
 */
PHP_METHOD(lis_model, where) 
{
    zval *where;
    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "|a", &where) == FAILURE) {
        return; 
    }

    zval *val, *a_val, *sql;
    zend_string *key, *a_key;
    smart_str buf = {0};
    int is_first = 0;
    int and_array_count = 0;
    int or_array_count = 0;

    sql = zend_read_property(lis_model_ce, getThis(), ZEND_STRL(LIS_MODEL_PROPERTY_NAME_SQL), 1, NULL);
    smart_str_appendl(&buf, Z_STRVAL_P(sql), Z_STRLEN_P(sql));
    smart_str_appendl(&buf, "WHERE", 5);

    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(where), key, val){
        if ((strcmp(ZSTR_VAL(key), "[OR]")) == 0 || (strcmp(ZSTR_VAL(key), "[AND]")) == 0) {

            if ((strcmp(ZSTR_VAL(key), "[OR]")) == 0) {
                smart_str_appendl(&buf, " OR", 3);
            } else {
                smart_str_appendl(&buf, " AND", 4);
            }

            and_array_count = zend_array_count(Z_ARRVAL_P(val));
            if (and_array_count > 1) {
                smart_str_appendl(&buf, " (", 2);
            }
            
            lis_model_and_logic(key, val, &buf, 0);

            if (and_array_count > 1) {
                smart_str_appendl(&buf, ") ", 2);
            }
        } else {
            is_first++;
            lis_model_where_analysis(key, val, &buf, is_first);
        }
    } ZEND_HASH_FOREACH_END();

    smart_str_0(&buf);
    zend_update_property_string(lis_model_ce, getThis(), ZEND_STRL(LIS_MODEL_PROPERTY_NAME_SQL), ZSTR_VAL(buf.s));
    smart_str_free(&buf);

    RETURN_ZVAL(getThis(), 1, 0);
}
/* }}} */

/* {{{ proto LisModel::columns(mixed $columns)
 */
PHP_METHOD(lis_model, columns)
{
    zval *columns, *table;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "|z", &columns) == FAILURE) {
        return; 
    }

    if ((table= zend_read_property(lis_model_ce, getThis(), ZEND_STRL(LIS_MODEL_PROPERTY_NAME_TABLE), 1, NULL)) == NULL) {
        php_error_docref(NULL, E_ERROR, "The table name cannot be empty");
    }

    smart_str buf = {0};
    zval *val;
    uint32_t columns_length;
    ulong idx;

    smart_str_appendl(&buf, "SELECT ", 7);
    if (IS_STRING == Z_TYPE_P(columns)) {
        smart_str_appendl(&buf, "* ", 2);
    } else if (IS_ARRAY == Z_TYPE_P(columns)) {
        columns_length = zend_array_count(Z_ARRVAL_P(columns));
       
        ZEND_HASH_FOREACH_NUM_KEY_VAL(Z_ARRVAL_P(columns), idx, val){
            smart_str_appendl(&buf, Z_STRVAL_P(val), Z_STRLEN_P(val)); 
            if (idx < (columns_length-1)) {
                smart_str_appendl(&buf, ",", 1);
            }
        } ZEND_HASH_FOREACH_END();

    } else {
        php_error_docref(NULL, E_ERROR, "Columns param failed");
        return;
    }

    smart_str_appendl(&buf, " FROM ", 6);
    smart_str_appendl(&buf, Z_STRVAL_P(table), Z_STRLEN_P(table));
    smart_str_appendl(&buf, " ", 1);

    smart_str_0(&buf);
    zend_update_property_string(lis_model_ce, getThis(), ZEND_STRL(LIS_MODEL_PROPERTY_NAME_SQL), ZSTR_VAL(buf.s));
    smart_str_free(&buf);

    RETURN_ZVAL(getThis(), 1, 0);
}
/* }}} */

/* {{{ proto LisModel::group(array $group)
 */
PHP_METHOD(lis_model, group)
{
    zval *group, *sql;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "a", &group) == FAILURE) {
        return; 
    }

    smart_str buf = {0};

    sql = zend_read_property(lis_model_ce, getThis(), ZEND_STRL(LIS_MODEL_PROPERTY_NAME_SQL), 1, NULL);
    smart_str_appendl(&buf, Z_STRVAL_P(sql), Z_STRLEN_P(sql));
    smart_str_appendl(&buf, " GROUP BY ", 10);

    zval *val;
    uint32_t group_length;
    ulong idx;
    group_length = zend_array_count(Z_ARRVAL_P(group));
       
    ZEND_HASH_FOREACH_NUM_KEY_VAL(Z_ARRVAL_P(group), idx, val){
        smart_str_appendl(&buf, Z_STRVAL_P(val), Z_STRLEN_P(val)); 
        if (idx < (group_length-1)) {
            smart_str_appendl(&buf, ",", 1);
        }
    } ZEND_HASH_FOREACH_END();

    smart_str_appendl(&buf, " ", 1);
    smart_str_0(&buf);
    zend_update_property_string(lis_model_ce, getThis(), ZEND_STRL(LIS_MODEL_PROPERTY_NAME_SQL), ZSTR_VAL(buf.s));
    smart_str_free(&buf);

    RETURN_ZVAL(getThis(), 1, 0);
}
/* }}} */

/* {{{ proto LisModel::order(array $order)
 */
PHP_METHOD(lis_model, order)
{
    zval *order, *sql;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "a", &order) == FAILURE) {
        return; 
    }

    smart_str buf = {0};

    sql = zend_read_property(lis_model_ce, getThis(), ZEND_STRL(LIS_MODEL_PROPERTY_NAME_SQL), 1, NULL);
    smart_str_appendl(&buf, Z_STRVAL_P(sql), Z_STRLEN_P(sql));
    smart_str_appendl(&buf, " ORDER BY", 9);

    zval *val;
    uint32_t order_length;
    int is_last = 0;
    zend_string *key;
    order_length = zend_array_count(Z_ARRVAL_P(order));
       
    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(order), key, val){
        is_last++;
        smart_str_appendl(&buf, " ", 1);
        smart_str_appendl(&buf, ZSTR_VAL(key), ZSTR_LEN(key)); 
        smart_str_appendl(&buf, " ", 1);

        smart_str_appendl(&buf, Z_STRVAL_P(val), Z_STRLEN_P(val)); 
        if (is_last < order_length) {
            smart_str_appendl(&buf, ",", 1);
        }
    } ZEND_HASH_FOREACH_END();

    smart_str_appendl(&buf, " ", 1);
    smart_str_0(&buf);
    zend_update_property_string(lis_model_ce, getThis(), ZEND_STRL(LIS_MODEL_PROPERTY_NAME_SQL), ZSTR_VAL(buf.s));
    smart_str_free(&buf);

    RETURN_ZVAL(getThis(), 1, 0);
}
/* }}} */

/* {{{ proto LisModel::prepare(mixed $sql)
 */
PHP_METHOD(lis_model, prepare)
{
    zval *sql, *pdo;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "z", &sql) == FAILURE) {
        return;
    }

    pdo = zend_read_static_property(lis_model_ce, ZEND_STRL(LIS_MODEL_PROPERTY_NAME_PDO), 1);
    if (pdo != NULL) {
        zval func, ret;

        zval params[1];
        ZVAL_STRING(&params[0], Z_STRVAL_P(sql));

        ZVAL_STRING(&func, "prepare");

        zend_fcall_info fci = {
            sizeof(fci), 
            func,
            &ret,
            params,
            Z_OBJ_P(pdo),
            1,
            1
        };

        if (zend_call_function(&fci, NULL) == FAILURE) {
            php_error_docref(NULL, E_ERROR, "Pdo prepare failed");
            RETURN_NULL();
        }

        zval_ptr_dtor(&func);
        zval_ptr_dtor(params);

        RETURN_ZVAL(&ret, 1, 0);
    }
    RETURN_NULL();
}
/* }}} */

/* {{{ proto LisModel::leftJoin(array $join)
 */
PHP_METHOD(lis_model, leftJoin)
{
    LIS_MODEL_JOIN(LEFT JOIN);
}
/* }}} */

/* {{{ proto LisModel::rightJoin(array $join)
 */
PHP_METHOD(lis_model, rightJoin)
{
    LIS_MODEL_JOIN(RIGHT JOIN);
}
/* }}} */

/* {{{ proto LisModel::fullJoin(array $join)
 */
PHP_METHOD(lis_model, fullJoin)
{
    LIS_MODEL_JOIN(FULL JOIN);
}
/* }}} */

/* {{{ proto LisModel::innerJoin(array $join)
 */
PHP_METHOD(lis_model, innerJoin)
{
    LIS_MODEL_JOIN(INNER JOIN);
}
/* }}} */

/* {{{ proto LisModel::insert(array $insert_data)
 */
PHP_METHOD(lis_model, insert)
{
    zval *insert_data, *table;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "a", &insert_data) == FAILURE) {
        return;
    }

    smart_str buf = {0};

    table = zend_read_property(lis_model_ce, getThis(), ZEND_STRL(LIS_MODEL_PROPERTY_NAME_TABLE), 1, NULL);
    smart_str_appendl(&buf, "INSERT INTO ", 12);
    smart_str_appendl(&buf, Z_STRVAL_P(table), Z_STRLEN_P(table));
    smart_str_appendl(&buf, "(", 1);

    zval *val;
    uint32_t insert_length;
    int is_last = 0;
    zend_string *key;
    insert_length = zend_array_count(Z_ARRVAL_P(insert_data));
       
    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(insert_data), key, val){
        is_last++;
        smart_str_appendl(&buf, ZSTR_VAL(key), ZSTR_LEN(key)); 

        if (is_last < insert_length) {
            smart_str_appendl(&buf, ",", 1);
        }
    } ZEND_HASH_FOREACH_END();

    smart_str_appendl(&buf, ")VALUES(", 8);

    is_last = 0;
    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(insert_data), key, val){
        is_last++;
        if (IS_STRING == Z_TYPE_P(val)) {
            smart_str_appendl(&buf, "'", 1);
            smart_str_appendl(&buf, Z_STRVAL_P(val), Z_STRLEN_P(val));
            smart_str_appendl(&buf, "'", 1);
        } else {
            convert_to_string_ex(val);
            smart_str_appendl(&buf, Z_STRVAL_P(val), Z_STRLEN_P(val));
        }

        if (is_last < insert_length) {
            smart_str_appendl(&buf, ",", 1);
        }
    } ZEND_HASH_FOREACH_END();

    smart_str_appendl(&buf, ")", 1);

    smart_str_0(&buf);
    zend_update_property_string(lis_model_ce, getThis(), ZEND_STRL(LIS_MODEL_PROPERTY_NAME_SQL), ZSTR_VAL(buf.s));
    smart_str_free(&buf);

    zval ret = {{0}};
    lis_model_query(getThis(), &ret);

    if (!ZVAL_IS_NULL(&ret)) {
        RETURN_ZVAL(&ret, 1, 0);
    }
    RETURN_FALSE;
}
/* }}} */

/* {{{ proto LisModel::update(array $set, array $where)
 */
PHP_METHOD(lis_model, update)
{
    zval *update_data, *where, *table;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "aa", &update_data, &where) == FAILURE) {
        return;
    }

    smart_str buf = {0};

    table = zend_read_property(lis_model_ce, getThis(), ZEND_STRL(LIS_MODEL_PROPERTY_NAME_TABLE), 1, NULL);
    smart_str_appendl(&buf, "UPDATE ", 7);
    smart_str_appendl(&buf, Z_STRVAL_P(table), Z_STRLEN_P(table));
    smart_str_appendl(&buf, " SET ", 5);

    zval *val;
    uint32_t length;
    int is_last = 0;
    int is_first = 0;
    zend_string *key;
    int and_array_count = 0;
    length = zend_array_count(Z_ARRVAL_P(update_data));
       
    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(update_data), key, val){
        is_last++;
        smart_str_appendl(&buf, ZSTR_VAL(key), ZSTR_LEN(key));
        smart_str_appendl(&buf, "=", 1);

        if (IS_STRING == Z_TYPE_P(val)) {
            smart_str_appendl(&buf, "'", 1);
            smart_str_appendl(&buf, Z_STRVAL_P(val), Z_STRLEN_P(val));
            smart_str_appendl(&buf, "'", 1);
        } else {
            convert_to_string_ex(val);
            smart_str_appendl(&buf, Z_STRVAL_P(val), Z_STRLEN_P(val));
        }

        if (is_last < length) {
            smart_str_appendl(&buf, ",", 1);
        }
    } ZEND_HASH_FOREACH_END();

    smart_str_appendl(&buf, " WHERE", 6);

    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(where), key, val){
        if ((strcmp(ZSTR_VAL(key), "[OR]")) == 0 || (strcmp(ZSTR_VAL(key), "[AND]")) == 0) {

            if ((strcmp(ZSTR_VAL(key), "[OR]")) == 0) {
                smart_str_appendl(&buf, " OR", 3);
            } else {
                smart_str_appendl(&buf, " AND", 4);
            }

            and_array_count = zend_array_count(Z_ARRVAL_P(val));
            if (and_array_count > 1) {
                smart_str_appendl(&buf, " (", 2);
            }
            
            lis_model_and_logic(key, val, &buf, 0);

            if (and_array_count > 1) {
                smart_str_appendl(&buf, ") ", 2);
            }
        } else {
            is_first++;
            lis_model_where_analysis(key, val, &buf, is_first);
        }
    } ZEND_HASH_FOREACH_END();

    smart_str_0(&buf);
    zend_update_property_string(lis_model_ce, getThis(), ZEND_STRL(LIS_MODEL_PROPERTY_NAME_SQL), ZSTR_VAL(buf.s));
    smart_str_free(&buf);

    zval ret = {{0}};
    lis_model_query(getThis(), &ret);

    if (!ZVAL_IS_NULL(&ret)) {
        RETURN_ZVAL(&ret, 1, 0);
    }
    RETURN_FALSE;
}
/* }}} */

/* {{{ proto LisModel::delete(array $where)
 */
PHP_METHOD(lis_model, delete)
{
    zval *where, *table;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "a", &where) == FAILURE) {
        return;
    }

    smart_str buf = {0};

    table = zend_read_property(lis_model_ce, getThis(), ZEND_STRL(LIS_MODEL_PROPERTY_NAME_TABLE), 1, NULL);
    smart_str_appendl(&buf, "DELETE FROM ", 12);
    smart_str_appendl(&buf, Z_STRVAL_P(table), Z_STRLEN_P(table));

    zval *val;
    int is_first = 0;
    int and_array_count = 0;
    zend_string *key;
       
    smart_str_appendl(&buf, " WHERE", 6);

    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(where), key, val){
        if ((strcmp(ZSTR_VAL(key), "[OR]")) == 0 || (strcmp(ZSTR_VAL(key), "[AND]")) == 0) {

            if ((strcmp(ZSTR_VAL(key), "[OR]")) == 0) {
                smart_str_appendl(&buf, " OR", 3);
            } else {
                smart_str_appendl(&buf, " AND", 4);
            }

            and_array_count = zend_array_count(Z_ARRVAL_P(val));
            if (and_array_count > 1) {
                smart_str_appendl(&buf, " (", 2);
            }
            
            lis_model_and_logic(key, val, &buf, 0);

            if (and_array_count > 1) {
                smart_str_appendl(&buf, ") ", 2);
            }
        } else {
            is_first++;
            lis_model_where_analysis(key, val, &buf, is_first);
        }
    } ZEND_HASH_FOREACH_END();

    smart_str_0(&buf);
    zend_update_property_string(lis_model_ce, getThis(), ZEND_STRL(LIS_MODEL_PROPERTY_NAME_SQL), ZSTR_VAL(buf.s));
    smart_str_free(&buf);

    zval ret = {{0}};
    lis_model_query(getThis(), &ret);

    if (!ZVAL_IS_NULL(&ret)) {
        RETURN_ZVAL(&ret, 1, 0);
    }
    RETURN_FALSE;
}
/* }}} */

/* {{{ proto LisModel::selectTable(string $table)
 */
PHP_METHOD(lis_model, selectTable) 
{
    zval *table;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "|z", &table) == FAILURE) {
        return; 
    }

    zend_update_property_string(lis_model_ce, getThis(), ZEND_STRL(LIS_MODEL_PROPERTY_NAME_SQL), "");
    zend_update_property_string(lis_model_ce, getThis(), ZEND_STRL(LIS_MODEL_PROPERTY_NAME_TABLE), Z_STRVAL_P(table));

    RETURN_TRUE;
}
/* }}} */

/* {{{ proto LisModel::table(string $table)
 */
PHP_METHOD(lis_model, table) 
{
    zval *table;

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "|z", &table) == FAILURE) {
        return; 
    }

    zend_update_property_string(lis_model_ce, getThis(), ZEND_STRL(LIS_MODEL_PROPERTY_NAME_SQL), "");
    zend_update_property_string(lis_model_ce, getThis(), ZEND_STRL(LIS_MODEL_PROPERTY_NAME_TABLE), Z_STRVAL_P(table));

    RETURN_ZVAL(getThis(), 1, 0);
}
/* }}} */

/* {{{ proto LisModel::getSql()
 */
PHP_METHOD(lis_model, getSql) 
{
    zval *sql;

    sql = zend_read_property(lis_model_ce, getThis(), ZEND_STRL(LIS_MODEL_PROPERTY_NAME_SQL), 1, NULL);

    RETURN_ZVAL(sql, 1, 0);
}
/* }}} */

/* {{{ proto LisModel::query()
 */
PHP_METHOD(lis_model, query)
{
    zval ret = {{0}};
    lis_model_query(getThis(), &ret);

    if (!ZVAL_IS_NULL(&ret)) {
        RETURN_ZVAL(&ret, 1, 0);
    }

    RETURN_NULL();
}
/* }}} */

/* {{{ proto LisModel::construct()
 */
PHP_METHOD(lis_model, __construct) 
{ 
    zval *db_config,*pdo;
    zval *dns, *user, *password, *option;

    lis_model_t *self = getThis();

    pdo = zend_read_static_property(lis_model_ce, ZEND_STRL(LIS_MODEL_PROPERTY_NAME_PDO), 1);

    if (UNEXPECTED(IS_OBJECT == Z_TYPE_P(pdo))) {
        return;
    }

    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "a", &db_config) == FAILURE) {
        RETURN_NULL();
    }

    if (IS_ARRAY == Z_TYPE_P(db_config)) {

        if ( (dns = zend_hash_str_find(Z_ARRVAL_P(db_config), ZEND_STRL(LIS_MODEL_PROPERTY_NAME_CONFIG_DNS))) != NULL) {
            zend_update_property(lis_model_ce, self, ZEND_STRL(LIS_MODEL_PROPERTY_NAME_DNS), dns);                        
        }

        if ( (user = zend_hash_str_find(Z_ARRVAL_P(db_config), ZEND_STRL(LIS_MODEL_PROPERTY_NAME_CONFIG_USER))) != NULL) {
            zend_update_property(lis_model_ce, self, ZEND_STRL(LIS_MODEL_PROPERTY_NAME_USER), user);                        
        }

        if ( (password = zend_hash_str_find(Z_ARRVAL_P(db_config), ZEND_STRL(LIS_MODEL_PROPERTY_NAME_CONFIG_PASSWORD))) != NULL) {
            zend_update_property(lis_model_ce, self, ZEND_STRL(LIS_MODEL_PROPERTY_NAME_PASSWORD), password);                        
        }

        if ( (option = zend_hash_str_find(Z_ARRVAL_P(db_config), ZEND_STRL(LIS_MODEL_PROPERTY_NAME_CONFIG_OPTION))) != NULL) {
            zend_update_property(lis_model_ce, self, ZEND_STRL(LIS_MODEL_PROPERTY_NAME_OPTION), option);                        
        }

        zend_update_property_string(lis_model_ce, getThis(), ZEND_STRL(LIS_MODEL_PROPERTY_NAME_SQL), "");

        zend_class_entry *pdo_ce;
        pdo_ce = zend_hash_str_find_ptr(EG(class_table), "pdo", 0x03);
        zval pdo_class = {{0}};

        if (pdo_ce != NULL) {
            object_init_ex(&pdo_class, pdo_ce);

            zval func, ret;
            zval z_option;

            ZVAL_COPY(&z_option, option);

            zval params[4];
            ZVAL_STRING(&params[0], Z_STRVAL_P(dns));
            ZVAL_STRING(&params[1], Z_STRVAL_P(user));
            ZVAL_STRING(&params[2], Z_STRVAL_P(password));
            params[3] = z_option;

            ZVAL_STRING(&func, "__construct");
            zend_fcall_info fci = {
                sizeof(fci), 
                func,
                &ret,
                params,
                Z_OBJ_P(&pdo_class),
                1,
                4
            };

            if (zend_call_function(&fci, NULL) == FAILURE) {
                php_error_docref(NULL, E_ERROR, "Pdo initialization failed");
                RETURN_NULL();
            }

            zval_ptr_dtor(&func);
            zval_ptr_dtor(params);

            zend_update_static_property(lis_model_ce, ZEND_STRL(LIS_MODEL_PROPERTY_NAME_PDO), &pdo_class);

        } else {
            php_error_docref(NULL, E_ERROR, "Pdo initialization failed");
            RETURN_NULL();
        }
    }
    RETURN_NULL();
}
/* }}} */

/* {{{ lis_model_methods
*/
zend_function_entry lis_model_methods[] = {
    PHP_ME(lis_model, __construct, lis_model_void_arginfo, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(lis_model, pdo, lis_model_void_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(lis_model, table, lis_model_table_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(lis_model, columns, lis_model_columns_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(lis_model, where, lis_model_where_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(lis_model, getSql, lis_model_void_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(lis_model, group, lis_model_group_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(lis_model, order, lis_model_order_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(lis_model, query, lis_model_void_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(lis_model, prepare, lis_model_prepare_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(lis_model, leftJoin, lis_model_join_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(lis_model, rightJoin, lis_model_join_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(lis_model, innerJoin, lis_model_join_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(lis_model, fullJoin, lis_model_join_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(lis_model, insert, lis_model_insert_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(lis_model, update, lis_model_update_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(lis_model, delete, lis_model_delete_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(lis_model, selectTable, lis_model_table_arginfo, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ LIS_STARTUP_FUNCTION
*/
LIS_STARTUP_FUNCTION(model) {
    zend_class_entry ce;

    LIS_INIT_CLASS_ENTRY(ce, "LisModel", "Lis\\Model", lis_model_methods);
    lis_model_ce = zend_register_internal_class_ex(&ce, NULL);
    lis_model_ce->ce_flags |= ZEND_ACC_EXPLICIT_ABSTRACT_CLASS; 

    zend_declare_property_null(lis_model_ce, ZEND_STRL(LIS_MODEL_PROPERTY_NAME_PDO), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
    zend_declare_property_null(lis_model_ce, ZEND_STRL(LIS_MODEL_PROPERTY_NAME_DNS), ZEND_ACC_PROTECTED);
    zend_declare_property_null(lis_model_ce, ZEND_STRL(LIS_MODEL_PROPERTY_NAME_USER), ZEND_ACC_PROTECTED);
    zend_declare_property_null(lis_model_ce, ZEND_STRL(LIS_MODEL_PROPERTY_NAME_PASSWORD), ZEND_ACC_PROTECTED);
    zend_declare_property_null(lis_model_ce, ZEND_STRL(LIS_MODEL_PROPERTY_NAME_OPTION), ZEND_ACC_PROTECTED);

    zend_declare_property_null(lis_model_ce, ZEND_STRL(LIS_MODEL_PROPERTY_NAME_SQL), ZEND_ACC_PUBLIC);

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


