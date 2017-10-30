/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_LIS_H
#define PHP_LIS_H

extern zend_module_entry lis_module_entry;
#define phpext_lis_ptr &lis_module_entry

#define PHP_LIS_VERSION "1.0.0" /* Replace with version number for your extension */
#define LIS_STARTUP_FUNCTION(module)    ZEND_MINIT_FUNCTION(lis_##module)
#define LIS_STARTUP(module)     ZEND_MODULE_STARTUP_N(lis_##module)(INIT_FUNC_ARGS_PASSTHRU)

#define lis_application_t   zval
#define lis_container_t     zval
#define lis_config_t        zval
#define lis_exception_t     zval
#define lis_router_t        zval
#define lis_loader_t        zval
#define lis_request_t       zval
#define lis_view_t          zval
#define lis_model_t         zval

#ifdef PHP_WIN32
#	define PHP_LIS_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_LIS_API __attribute__ ((visibility("default")))
#else
#	define PHP_LIS_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(lis)
    zend_bool running;
    zend_bool use_namespace;
	char *environ;
    zend_string *base_uri;
    zend_string *directory;
    zend_bool hit_router;
    zend_bool hit_group_router;
    zend_bool hit_action;
ZEND_END_MODULE_GLOBALS(lis)

/* Always refer to the globals in your function as LIS_G(variable).
   You are encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define LIS_G(v) TSRMG(lis_globals_id, zend_lis_globals *, v)
#else
#define LIS_G(v) (lis_globals.v)
#endif

#if defined(ZTS) && defined(COMPILE_DL_LIS)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

PHP_MINIT_FUNCTION(lis);
PHP_MSHUTDOWN_FUNCTION(lis);
PHP_RINIT_FUNCTION(lis);
PHP_RSHUTDOWN_FUNCTION(lis);
PHP_MINFO_FUNCTION(lis);
extern ZEND_DECLARE_MODULE_GLOBALS(lis);

#endif	/* PHP_LIS_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
