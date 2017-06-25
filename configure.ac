#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([jamyxer], [1.0], [javi.po.123@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([src/package_config.h])

LDFLAGS="$LDFLAGS -ljack -ljsoncpp -lreadline -pthread"

# AC_ARG_VAR([config], [Config filetype])
# AM_CONDITIONAL([XML_CONF],  [test $config = "xml"])
# AM_CONDITIONAL([YAML_CONF], [test $config = "yaml"])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
# AC_PROG_AWK
# AC_PROG_INSTALL
# AC_PROG_MKDIR_P

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
