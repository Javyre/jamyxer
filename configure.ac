#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([jamyxer], [1.0], [javi.po.123@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([src/package_config.h])

LDFLAGS="$LDFLAGS -ljack -ljsoncpp -pthread"

AC_ARG_WITH([readline],
            [AS_HELP_STRING([--with-readline], [use libreadline for better interactive cmd-line])],
            CPPFLAGS="$CPPFLAGS -DWITH_READLINE"
            LDFLAGS="$LDFLAGS -lreadline")

AC_ARG_ENABLE([debug],
              [AS_HELP_STRING([--enable-debug], [enable DEBUG flag in compilation])],
              CPPFLAGS="$CPPFLAGS -DDEBUG")

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

if [test $CXX = "clang++"]; then
    echo "$CXX compiler set... using -std=c++11..."
    CXXFLAGS="$CXXFLAGS -std=c++11"
fi

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
