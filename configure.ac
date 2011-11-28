#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([grapefruit], [0.0])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CC

AC_CHECK_HEADERS([stdlib.h string.h])

AC_HEADER_STDBOOL

AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_ERROR_AT_LINE

AC_TYPE_UINT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_UINT32_T

AC_CONFIG_FILES([
	Makefile
	src/Makefile
])

CFLAGS="-Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -Wundef -Wredundant-decls -Wpointer-arith"

AC_MSG_CHECKING(whether to enable debugging)
AC_ARG_ENABLE(debug,
	AS_HELP_STRING([--enable-debug], [enable debugging]),,
	enable_debug="no"
)

case "${enable_debug}" in
	yes)
		CFLAGS="$CFLAGS -g -O0 -DDEBUG"
		AC_MSG_RESULT(yes)
		;;
	no)
		CFLAGS="$CFLAGS -g0 -O3"
		AC_MSG_RESULT(no)
		;;
	*)
		AC_MSG_ERROR([bad value ${enable_debug} for --enable-debug])
		;;
esac

AC_OUTPUT
