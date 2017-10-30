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
  | Author: shendegang <php_shen@163.com                                 |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"

#include "php_lis.h"
#include "lis_exception.h"
#include "lis_application.h"

ZEND_DECLARE_MODULE_GLOBALS(lis)

PHP_INI_BEGIN() /* {{{ */
    STD_PHP_INI_ENTRY("lis.environ",            "develop", PHP_INI_SYSTEM, OnUpdateString, environ, zend_lis_globals, lis_globals)
    STD_PHP_INI_BOOLEAN("lis.use_namespace",    "0", PHP_INI_SYSTEM, OnUpdateBool, use_namespace, zend_lis_globals, lis_globals)
PHP_INI_END();
/* }}} */

PHP_GINIT_FUNCTION(lis) /* {{{ */
{
    memset(lis_globals, 0, sizeof(*lis_globals));
}
/* }}} */

PHP_MINIT_FUNCTION(lis) /* {{{ */
{
	REGISTER_INI_ENTRIES();

    if (LIS_G(use_namespace)) {
        REGISTER_STRINGL_CONSTANT("LIS\\VERSION", PHP_LIS_VERSION, sizeof(PHP_LIS_VERSION) - 1, CONST_PERSISTENT | CONST_CS);
    } else {
        REGISTER_STRINGL_CONSTANT("LIS_VERSION", PHP_LIS_VERSION, sizeof(PHP_LIS_VERSION) - 1, CONST_PERSISTENT | CONST_CS);
    }

    LIS_STARTUP(config);
    LIS_STARTUP(container);
    LIS_STARTUP(exception);
    LIS_STARTUP(router);
    LIS_STARTUP(application);
    LIS_STARTUP(request);
    LIS_STARTUP(loader);
    LIS_STARTUP(model);
    LIS_STARTUP(view);

    return SUCCESS;
}
/* }}} */

PHP_MSHUTDOWN_FUNCTION(lis) /* {{{ */
{
    UNREGISTER_INI_ENTRIES();
    return SUCCESS;
}
/* }}} */

PHP_RINIT_FUNCTION(lis) /* {{{ */
{
#if defined(COMPILE_DL_LIS) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    return SUCCESS;
}
/* }}} */

PHP_RSHUTDOWN_FUNCTION(lis) /* {{{ */
{

    LIS_G(running) = 0;
    LIS_G(hit_router) = 0;
    LIS_G(hit_group_router) = 0;
    LIS_G(hit_action) = 0;

    if (LIS_G(base_uri)) {
        zend_string_release(LIS_G(base_uri));
        LIS_G(base_uri) = NULL;
    }

    if (LIS_G(directory)) {
        zend_string_release(LIS_G(directory));
        LIS_G(directory) = NULL;
    }

    return SUCCESS;
}
/* }}} */

PHP_MINFO_FUNCTION(lis) /* {{{ */
{
    php_info_print_table_start();
    php_info_print_table_row(2, "Version", PHP_LIS_VERSION);
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}
/* }}} */

/** {{{ module depends
 */
#if ZEND_MODULE_API_NO >= 20050922
zend_module_dep lis_deps[] = {
    ZEND_MOD_REQUIRED("spl")
    ZEND_MOD_REQUIRED("pcre")
    ZEND_MOD_REQUIRED("standard")
    {NULL, NULL, NULL}
};
#endif
/* }}} */

/* {{{ lis_functions[]
 *
 * Every user visible function must have an entry in lis_functions[].
 */
const zend_function_entry lis_functions[] = {
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ lis_module_entry
 */
zend_module_entry lis_module_entry = {
    STANDARD_MODULE_HEADER_EX,
    NULL,
    lis_deps,
    "lis",
    lis_functions,
    PHP_MINIT(lis),
    PHP_MSHUTDOWN(lis),
    PHP_RINIT(lis),         /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(lis),     /* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(lis),
    PHP_LIS_VERSION,
    PHP_MODULE_GLOBALS(lis),
    PHP_GINIT(lis),
    NULL,
    NULL,
    STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_LIS
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(lis)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
