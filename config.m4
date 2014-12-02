PHP_ARG_ENABLE(couchbase, whether to enable Couchbase support,
[ --with-couchbase   Include Couchbase support])
if test "$PHP_COUCHBASE" = "yes"; then
  AC_DEFINE(HAVE_COUCHBASE, 1, [Whether you have Couchbase])

  AC_CHECK_HEADERS([libcouchbase/couchbase.h])
  AS_IF([test "x$ac_cv_header_libcouchbase_couchbase_h" = "xno"], [
		 AC_MSG_ERROR([the couchbase extension requires libcouchbase])])

  PHP_ADD_LIBRARY(couchbase, 1, COUCHBASE_SHARED_LIBADD)
  
  ifdef([PHP_ADD_EXTENDION_DEP], [
	PHP_ADD_EXTENSION_DEP(couchbase, json)
  ]) 

  PHP_SUBST(COUCHBASE_SHARED_LIBADD)
  PHP_NEW_EXTENSION(couchbase, \
	bucket.c \
	cas.c \
	cluster.c \
	couchbase.c \
	exception.c \
	metadoc.c \
	transcoding.c \
  , $ext_shared)
fi