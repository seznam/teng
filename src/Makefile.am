#
# Teng -- a general purpose templating engine.
# Copyright (C) 2004  Seznam.cz, a.s.
#
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
# $Id: Makefile.am,v 1.10 2008-11-20 23:56:31 burlog Exp $
#
# DESCRIPTION
# A makefile template for Teng.
#
# AUTHORS
# Vaclav Blazek <blazek@firma.seznam.cz>
#
# HISTORY
# 2003-09-17  (vasek)
#             Created.
#

AUTOMAKE_OPTIONS = subdir-objects

# warn on all
AM_CXXFLAGS = -Wall

# extra compiler options
AM_CPPFLAGS = -D_ISOC99_SOURCE -fPIC ${DEPENS_CFLAGS}                          \
			  -DSRC_DIR="\"$(realpath @abs_top_srcdir@)\""                     \
			  -I$(top_srcdir)/include -I$(top_builddir)/include

# these are generated and thus ought to be prepared before anything else
BUILT_SOURCES = lex2impl.cc lex2impl.h syntax.cc syntax.hh

# compile this library
lib_LTLIBRARIES = libteng.la

# from the these sources
libteng_la_SOURCES = teng.cc fragment.cc fragmentlist.cc util.cc               \
					 fragmentvalue.cc cache.cc sourcelist.cc dictionary.cc     \
					 instruction.cc value.cc parsercontext.cc program.cc       \
					 function.cc processor.cc writer.cc formatter.cc           \
					 contenttype.cc error.cc template.cc lex1.cc syntax.yy     \
					 position.cc lex2impl.ll udf.cc yystype.cc md5.cc          \
					 configuration.cc aux.cc logging.cc semantic.cc hex.cc     \
					 utf8.cc stringview.cc lex2.cc parserfrag.cc parserdiag.cc

# with these flags (version info etc.)
libteng_la_LDFLAGS = @VERSION_INFO@
libteng_la_LIBADD = $(LTLIBOBJS) ${DEPENS_LIBS}

# explicit rules for proper flex & bison sources compilation
lex2.lo: lex2impl.cc
parsercontext.lo: syntax.cc

# test program
EXTRA_PROGRAMS = teng-example teng-cache test-teng
teng_example_SOURCES = example/example.cc
teng_example_LDADD = libteng.la
teng_cache_SOURCES = example/cache.cc
teng_cache_LDADD = libteng.la

CLEANFILES = @top_builddir@/src/syntax.hh                                      \
			 @top_builddir@/src/syntax.cc                                      \
			 @top_builddir@/src/lex2impl.cc                                    \
			 @top_builddir@/src/lex2impl.h

doc:
	doxygen

test_teng_LDADD = libteng.la
test_teng_SOURCES = tests/main.cc tests/old.cc tests/fun-escaping.cc           \
					tests/frag.cc tests/vars.cc tests/simple.cc tests/incl.cc  \
					tests/format.cc tests/expr-string.cc tests/expr-int.cc     \
					tests/expr-real.cc tests/expr-other.cc tests/dict.cc       \
					tests/expr-regex.cc tests/ctype.cc tests/cond.cc           \
					tests/expr-tern.cc tests/expr-case.cc tests/fun-date.cc    \
					tests/fun-number.cc tests/fun-string.cc tests/fun-other.cc \
					tests/queries.cc tests/debug.cc tests/rtvars.cc            \
					tests/builtin-vars.cc

AM_COLOR_TESTS = always
TESTS = test-teng

