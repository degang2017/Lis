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

#ifndef LIS_LOADER_H
#define LIS_LOADER_H

#define LIS_LOADER_PROPERTY_NAME_INSTANCE   "_instance"
#define LIS_AUTOLOAD_FUNC_NAME              "autoload"
#define LIS_SPL_AUTOLOAD_REGISTER_NAME      "spl_autoload_register"

#define LIS_LOADER_MODEL                    "Model"
#define LIS_LOADER_LEN_MODEL                5
#define LIS_LOADER_CONTROLLER               "Controller"
#define LIS_LOADER_LEN_CONTROLLER           10
#define LIS_LOADER_PLUGIN                   "plugin"
#define LIS_LOADER_LEN_PLUGIN               6
#define LIS_LOADER_LIBRARY                  "library"
#define LIS_LOADER_LEN_LIBRARY              7

#define LIS_DIRECTORY_NAME_MODEL            "model"
#define LIS_DIRECTORY_NAME_CONTROLLER       "controller"
#define LIS_DIRECTORY_NAME_LIBRARY          "library"
#define LIS_DIRECTORY_NAME_PLUGIN           "plugin"
#define LIS_DIRECTORY_NAME_VIEW             "view"

#define LIS_EXT_NAME                        "php"
#define LIS_EXT_LEN_NAME                    3

extern zend_class_entry *lis_loader_ce;
void lis_loader_instance();
int lis_loader_import(zend_string *path, int use_path);
static int lis_loader_register();
static int lis_loader_autoload(char *file_name, size_t name_length, char **directory);

LIS_STARTUP_FUNCTION(loader);
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
