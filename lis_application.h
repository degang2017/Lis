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

#ifndef LIS_APPLICATION_H
#define LIS_APPLICATION_H

#define LIS_APPLICATION_PROPERTY_NAME_RUN       "_running"
#define LIS_APPLICATION_PROPERTY_NAME_ENV       "_environ"

#define LIS_INIT_CLASS_ENTRY(ce, name, namespace_name, methods) \
    if (LIS_G(use_namespace)) { \
        INIT_CLASS_ENTRY(ce, namespace_name, methods); \
    } else { \
        INIT_CLASS_ENTRY(ce, name, methods); \
    }

extern zend_class_entry *lis_application_ce;

LIS_STARTUP_FUNCTION(application);
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
