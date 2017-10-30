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

#ifndef LIS_MODEL_H
#define LIS_MODEL_H

#define LIS_MODEL_PROPERTY_NAME_PDO                 "_pdo"
#define LIS_MODEL_PROPERTY_NAME_USER                "_username"
#define LIS_MODEL_PROPERTY_NAME_PASSWORD            "_password"
#define LIS_MODEL_PROPERTY_NAME_DNS                 "_dns"
#define LIS_MODEL_PROPERTY_NAME_OPTION              "_option"
#define LIS_MODEL_PROPERTY_NAME_SQL                 "_sql"
#define LIS_MODEL_PROPERTY_NAME_TABLE               "table"

#define LIS_MODEL_PROPERTY_NAME_CONFIG_USER         "username"
#define LIS_MODEL_PROPERTY_NAME_CONFIG_PASSWORD     "password"
#define LIS_MODEL_PROPERTY_NAME_CONFIG_DNS          "dns"
#define LIS_MODEL_PROPERTY_NAME_CONFIG_OPTION       "option"

#define LIS_MODEL_JOIN(x) \
    do {\
        zval *join; \
        if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "a", &join) == FAILURE) { \
            return; \
        } \
        lis_model_join(join, " "#x" ", getThis()); \
        RETURN_ZVAL(getThis(), 1, 0); \
    } while(0)

extern zend_class_entry *lis_model_ce;
void lis_model_instance();

LIS_STARTUP_FUNCTION(model);
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
