#
# Teng -- a general purpose templating engine.
# Copyright (C) 2004  Seznam.cz, a.s.

# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Seznam.cz, a.s.
# Naskove 1, Praha 5, 15000, Czech Republic
# http://www.seznam.cz, mailto:teng@firma.seznam.cz
#
#
# $Id: configure.in,v 1.24 2010-06-11 07:46:26 burlog Exp $
#
# DESCRIPTION
# configure template for Teng.
#
# AUTHORS
# Vaclav Blazek <blazek@firma.seznam.cz>
#
# HISTORY
# 2003-09-17  (vasek)
#             Created.
#


# initialize autoconf
AC_INIT(teng, [3.0.0], opensource@firma.seznam.cz)

AC_CONFIG_SRCDIR([src/teng.cc])

AM_INIT_AUTOMAKE

AC_PROG_LIBTOOL

AC_CONFIG_MACRO_DIRS([m4])

AC_PREREQ(2.50)

#
AM_SILENT_RULES

# tell autoconf where to find sources
AC_CONFIG_SRCDIR([src/teng.cc])
AC_CONFIG_FILES([tests/Makefile])

# check for C++ compiler
AC_PROG_CXX
AC_LANG_CPLUSPLUS
# needs autoconf-archive. Install that if you're getting
# error: possibly undefined macro: AC_MSG_ERROR
AX_CHECK_COMPILE_FLAG([-std=c++11], [], [AC_MSG_ERROR([Compiler without c++11])])

# pkg config
PKG_PROG_PKG_CONFIG([0.25])

DEPENS_NAMES="glib-2.0 libcurl libpcre2-8"
PKG_CHECK_MODULES([DEPENS], [${DEPENS_NAMES}])

AC_SUBST(DEPENS_NAMES)
AC_SUBST(DEPENS)



# check for std C headers
AC_HEADER_STDC

# check for libtool
AC_PROG_LIBTOOL

# This version number needs to be changed in several different ways for each
# release. Please read the libtool documentation (info libtool 'Updating
# version info') before touching this. (this is *.so version number).
TENG_MAJOR=3
TENG_MINOR=0
VERSION_INFO="-version-info $TENG_MAJOR:$TENG_MINOR:0"
AC_SUBST(TENG_MAJOR)
AC_SUBST(TENG_MINOR)
AC_SUBST(VERSION_INFO)


CPPFLAGS=""
CXXFLAGS=""
CFLAGS=""
LDFLAGS=""

AC_ARG_ENABLE(optimization,[  --enable-optimization compile optimized without debug logging],[
    case "${enableval}" in
        no)
            AC_MSG_NOTICE([disabling optimization.])
            CPPFLAGS="${CPPFLAGS} -DDEBUG=1 -O0 -g -ggdb"
        ;;
        yes)
            AC_MSG_NOTICE([enabling optimization.])
            CPPFLAGS="${CPPFLAGS} -O2 -g -ggdb"
        ;;
        *)
            AC_MSG_ERROR([Say yes or no to --enable-optimization.])
        ;;
    esac
], [
   CPPFLAGS="${CPPFLAGS} -O2 -g -ggdb"
])

AC_ARG_ENABLE(lock-udf,[ --disable-udf-locks compile without thread safe UDF],[
    case "${enableval}" in
        no)
            AC_MSG_NOTICE([enabling UDF locks])
            CPPFLAGS="${CPPFLAGS} -UNO_UDF_LOCKS"
            check_pthreads="no"
        ;;
        yes)
            AC_MSG_NOTICE([enabling optimization.])
            CPPFLAGS="${CPPFLAGS} -DNO_UDF_LOCKS=1"
            check_pthreads="yes"
        ;;
        *)
            AC_MSG_ERROR([Say yes or no to --disable-udf-locks])
        ;;
    esac
], [
   CPPFLAGS="${CPPFLAGS} -UNO_UDF_LOCKS"
    check_pthreads="yes"
])

# check for flex and bison
AM_PROG_LEX
AC_PROG_YACC

AX_BOOST_BASE([1.48],, [AC_MSG_ERROR([tests needs Boost, but it was not found in your system])])
AX_BOOST_SYSTEM
AX_BOOST_FILESYSTEM
AX_BOOST_UNIT_TEST_FRAMEWORK
AC_SUBST(BOOST_FILESYSTEM_LIB)
AC_SUBST(BOOST_SYSTEM_LIB)
AC_SUBST(BOOST_UNIT_TEST_FRAMEWORK_LIB)

# check for special header files
AC_CHECK_HEADERS(fenv.h)

# check for functions that may miss on some platforms
AC_CHECK_LIB(m, floor)
AC_REPLACE_FUNCS([trunc round])

AX_CHECK_COMPILE_FLAG([-std=c++14], [
    check_cpp14="true"
    AC_MSG_NOTICE([Compiler with c++14, test are enabled.])
],[AC_MSG_NOTICE([Compiler without c++14, test are disabled.])])

AM_CONDITIONAL([CPP14], [test x$check_cpp14 == xtrue])

AS_IF([test x$check_pthreads == xyes], [
    # Check for nonglibc header from serr library. The library is not
    # needed just this header.
    AC_CHECK_LIB(pthread, pthread_rwlock_rdlock,, AC_MSG_ERROR([pthread_rwlock_rdlock not found]))
], [])

# generate Makefile
AC_OUTPUT(version Makefile src/Makefile src/tengconfig.h libteng.pc)
#          doc/Makefile doc/manual/Makefile doc/manual/en/Makefile)
