#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([grapefruit], [0.1])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CC
AC_PROG_CC_C99

AC_C_INLINE

AC_CHECK_HEADERS([stdlib.h string.h])

AC_HEADER_STDBOOL

AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_ERROR_AT_LINE

AC_CONFIG_FILES([
	Makefile
	src/Makefile
])

AC_OUTPUT
