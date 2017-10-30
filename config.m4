dnl $Id$
dnl config.m4 for extension lis

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(lis, for lis support,
[  --with-lis             Include lis support])

PHP_ARG_ENABLE(lis, whether to enable lis support,
[  --enable-lis           Enable lis support])

if test "$PHP_LIS" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-lis -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/lis.h"  # you most likely want to change this
  dnl if test -r $PHP_LIS/$SEARCH_FOR; then # path given as parameter
  dnl   LIS_DIR=$PHP_LIS
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for lis files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       LIS_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$LIS_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the lis distribution])
  dnl fi

  dnl # --with-lis -> add include path
  dnl PHP_ADD_INCLUDE($LIS_DIR/include)

  dnl # --with-lis -> check for lib and symbol presence
  dnl LIBNAME=lis # you may want to change this
  dnl LIBSYMBOL=lis # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $LIS_DIR/$PHP_LIBDIR, LIS_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_LISLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong lis lib version or lib not found])
  dnl ],[
  dnl   -L$LIS_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(LIS_SHARED_LIBADD)

  PHP_NEW_EXTENSION(lis,
  lis.c                     \
  lis_application.c         \
  lis_exception.c           \
  lis_config.c              \
  lis_request.c             \
  lis_loader.c              \
  lis_router.c              \
  lis_view.c                \
  lis_model.c               \
  lis_container.c,
  $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi

if test -z "$PHP_DEBUG"; then   
    AC_ARG_ENABLE(debug,  
    [   --enable-debug          compile with debugging symbols],[  
        PHP_DEBUG=$enableval  
    ],[ PHP_DEBUG=no  
    ])  
fi  
