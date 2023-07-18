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
AC_INIT([teng], [5.0.4~burloglibteng4-1], [teng@firma.seznam.cz])
AC_LANG([C++])
# https://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
VERSION_INFO="-version-info 5:2:0"

# enable silent builds (disable with --disable-silent-rules)
# or simply do "make V=1"
AM_SILENT_RULES([yes])

# install autotools scripts to build-aux dir
AC_CONFIG_AUX_DIR([build-aux])

# macros from http://www.gnu.org/software/autoconf-archive/The-Macros.html
AC_CONFIG_MACRO_DIR([m4])

# initialize automake(use AC_INIT's arguments)
AM_INIT_AUTOMAKE(-Wall -Wno-portability foreign color-tests)

# tell autoconf where to find sources
AC_CONFIG_SRCDIR([src/teng.cc])

# this forces debian autoconf to use version 2.53
AC_PREREQ([2.71])

# replace autotools original defaults
: ${CXXFLAGS="-W -Wall -Wextra -Wconversion -Wno-sign-conversion -fPIC -fno-omit-frame-pointer -g -ggdb -std=c++14"}
: ${LDFLAGS="-pthread -rdynamic"}

# cxx flags for libfuzzer
SANITIZE_CXXFLAGS="-fsanitize=undefined -fsanitize=address"
CODE_COVERAGE_CXXFLAGS="-fprofile-instr-generate -fcoverage-mapping"
FUZZER_CXXFLAGS="${SANITIZE_CXXFLAGS} -fsanitize=fuzzer"

# --enable-package-version switch
AC_ARG_ENABLE(
    [package-version],
    [AS_HELP_STRING([--enable-package-version], [compile with specific package version])],
    [AS_IF(
        [test x${enableval} = xyes],
        [AC_MSG_ERROR([You must specify package version])],
        [AS_IF(
            [test x${enableval} = xyes],
            [AC_DEFINE([DEBIAN_PACKAGE_VERSION], "\\\"${enableval}\\\"", [package version])]
        )]
    )]
)

# --enable-optimization switch
AC_ARG_ENABLE(
    [optimization],
    [AS_HELP_STRING([--enable-optimization], [compile optimized])],
    [optimization=${enableval}; AS_IF([test x${enableval} = xyes], [CXXFLAGS="${CXXFLAGS} -O3"], [CXXFLAGS="${CXXFLAGS} -O0"])],
    [optimization=no; CXXFLAGS="${CXXFLAGS} -O0"]
)

# --enable-debug switch
AC_ARG_ENABLE(
    [debug],
    [AS_HELP_STRING([--enable-debug], [enable the debug build])],
    [AS_IF([test x${enableval} = xyes], [AC_DEFINE([DEBUG], 1, [allow debug build])])]
)

# --enable-sanitize switch
AC_ARG_ENABLE(
    [sanitize],
    [AS_HELP_STRING([--enable-sanitize], [enable the sanitizer cxx flags])],
    [AS_IF([test x${enableval} = xyes], [CXXFLAGS="${CXXFLAGS} ${SANITIZE_CXXFLAGS}"])]
)

# --enable-stl-debug switch
AC_ARG_ENABLE(
    [stl-debug],
    [AS_HELP_STRING([--enable-stl-debug], [enable the STL debug version])],
    [AS_IF([test x${enableval} = xyes], [CXXFLAGS="${CXXFLAGS} -D_GLIBCXX_DEBUG"])]
)

# --enable-code-coverage switch
AC_ARG_ENABLE(
    [code-coverage],
    [AS_HELP_STRING([--enable-code-coverage], [enable the code coverage cxx flags])],
    [AS_IF([test x${enableval} = xyes], [CXXFLAGS="${CXXFLAGS} ${CODE_COVERAGE_CXXFLAGS}"])]
)

# --disable-udf-lock switch
AC_ARG_ENABLE(
    [lock-udf],
    [AS_HELP_STRING([--enable-lock-udf], [enable locking of udf registry])],
    [AS_IF([test x${enableval} = xno], [AC_DEFINE([NO_UDF_LOCKS], 1, [disable locking of udf registry])])]
)

# --enable-fuzzer switch
AC_ARG_ENABLE(
   [fuzzer],
   [AS_HELP_STRING([--enable-fuzzer], [compile with llvm libfuzzer])],
   [FUZZER=${enableval}; AS_IF([test x${enableval} = xyes], [CXXFLAGS="${CXXFLAGS} ${FUZZER_CXXFLAGS}"; AC_DEFINE([FUZZER])])],
   [FUZZER=no]
)
AM_CONDITIONAL([FUZZER], [test x${FUZZER} = xyes])

# the AC_PROG_CC can't be in conditionals, so we are forced to replace default compiler list
# see https://bug-autoconf.gnu.narkive.com/bTkajPb5/ac-prog-cc-does-not-work-correctly-inside-conditionals-autoconf-2-61
AS_IF([test x${FUZZER} = xyes], [CC_LIST="clang"], [CC_LIST="gcc clang c1 cc"])
AS_IF([test x${FUZZER} = xyes], [CXX_LIST="clang++"], [CXX_LIST="g++ clang++ cxx cc++ c++ g++"])

# check for C++ compiler
AC_PROG_CC([$CC_LIST])
AC_PROG_CXX([$CXX_LIST])
LT_INIT
AX_CHECK_COMPILE_FLAG([-std=c++14], [],[AC_MSG_ERROR([Compiler without c++14])])

# check for flex and bison
AC_PROG_LEX([noyywrap])
AC_PROG_YACC

# check for special header files
AC_CHECK_HEADERS(fenv.h)

# check for functions that may miss on some platforms
AC_CHECK_LIB(m, floor)
AC_REPLACE_FUNCS([trunc round])

# check pkgconfig
PKG_PROG_PKG_CONFIG([0.25])
TENG_PKG_CONFIG_MODULES=m4_flatten("
    glib-2.0
    libpcre2-8
")
PKG_CHECK_MODULES([TENG_DEPS], [$TENG_PKG_CONFIG_MODULES])

# define this variables in Makefile.am
AC_SUBST(GIT_REVISION)
AC_SUBST(VERSION_INFO)
AC_SUBST(TENG_DEPS)
AC_SUBST(TENG_PKG_CONFIG_MODULES)

# generated files
AC_CONFIG_FILES([
    Makefile
    src/Makefile
    include/teng/Makefile
    include/teng/config.h
    libteng.pc
    version
])
AC_OUTPUT

