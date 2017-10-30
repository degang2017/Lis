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

#ifndef LIS_EXCEPTION_H
#define LIS_EXCEPTION_H

#define LIS_MAX_SYS_EXCEPTION 10
#define LIS_ERR_MASK 127
#define LIS_EXCEPTION_PROPERTY_NAME_INSTANCE "_instance"

#define LIS_ERR_BASE 512
#define LIS_ERR_CONTROLLER_FAILED   513
#define LIS_ERR_MIDDLEWARE_FAILED   514
#define LIS_ERR_VIEW_FAILED         515

#define LIS_EXCEPTION_OFFSET(n) (n & LIS_ERR_MASK)

extern zend_class_entry *lis_exception_ce;

extern zend_class_entry *lis_ce_RuntimeException;
extern zend_class_entry *lis_sys_exceptions[LIS_MAX_SYS_EXCEPTION];
void lis_throw_exception(int type, char *format, ...);

LIS_STARTUP_FUNCTION(exception);
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
