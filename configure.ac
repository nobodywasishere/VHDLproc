AC_PREREQ(2.69)

AC_INIT([vpp], [2.0.3d], [takashige@users.sourceforge.net])
AC_CONFIG_SRCDIR([src/vpp.h])
AC_PREFIX_DEFAULT([/usr])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_SILENT_RULES([yes])

AC_USE_SYSTEM_EXTENSIONS
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_MAKE_SET
AM_PROG_AR
AC_PROG_LIBTOOL
AC_PROG_INSTALL

AC_DEFINE(FALSE, (0), [Boolean False])
AC_DEFINE(TRUE, (!FALSE), [Boolean True])

AC_CHECK_HEADER(stdio.h, , AC_MSG_ERROR([Can't find stdio.h]))
AC_CHECK_HEADER(stdlib.h, , AC_MSG_ERROR([Can't find stdlib.h]))
AC_CHECK_HEADER(string.h, , AC_MSG_ERROR([Can't find string.h]))
AC_CHECK_HEADER(limits.h, , AC_MSG_ERROR([Can't find limits.h]))
AC_CHECK_HEADER([sys/stat.h], , AC_MSG_ERROR([Can't find sys/stat.h]))
AC_CHECK_HEADER([sys/types.h], , AC_MSG_ERROR([Can't find sys/types.h]))
AC_CHECK_HEADER(time.h, , AC_MSG_ERROR([Can't find time.h]))
AC_CHECK_HEADER(getopt.h, , AC_MSG_ERROR([Can't find getopt.h]))
AC_CHECK_HEADER(unistd.h, , AC_MSG_ERROR([Can't find unistd.h]))
AC_CHECK_HEADER(iconv.h, , AC_MSG_ERROR([Can't find iconv.h]))
AC_CHECK_HEADER(errno.h, , AC_MSG_ERROR([Can't find errno.h]))

AC_ARG_WITH(lbs, [  --with-lbs=SIZE[[byte]]   line buffer size [[default=4096]]], lbs="$withval", lbs=4096)
AC_DEFINE_UNQUOTED(LINE_BUFFER_SIZE, ${lbs}, [configured line buffer size])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
