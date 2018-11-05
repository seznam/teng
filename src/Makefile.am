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
AM_CPPFLAGS = -D_ISOC99_SOURCE -fPIC ${DEPENS_CFLAGS} \
	      -DSRC_DIR="\"$(realpath @abs_top_srcdir@)\""

# these are generated and thus ought to be prepared before anything else
BUILT_SOURCES = tenglex2impl.cc tenglex2impl.h tengsyntax.cc tengsyntax.hh

# install these headers
include_HEADERS = teng.h tengstructs.h tengwriter.h tengerror.h tengconfig.h \
		  tengudf.h tengfragment.h tengfragmentlist.h \
		  tengfragmentvalue.h

noinst_HEADERS = tengcache.h tengcontenttype.h tengdictionary.h \
		 tengformatter.h tengfunction.h tenginstruction.h tenglex1.h \
		 tengparsercontext.h tengvalue.h tengprocessor.h \
		 tengprogram.h tengsourcelist.h tengtemplate.h \
		 tengutil.h tengwriter.h tengsyntax.hh tengconfiguration.h \
		 tengplatform.h tengaux.h tengposition.h

# compile this library
lib_LTLIBRARIES = libteng.la

# from the these sources
libteng_la_SOURCES = teng.cc tengfragment.cc tengfragmentlist.cc tengutil.cc \
		     tengfragmentvalue.cc tengcache.cc tengsourcelist.cc \
		     tengdictionary.cc tenginstruction.cc tengvalue.cc \
		     tengparsercontext.cc tengprogram.cc tengfunction.cc \
		     tengprocessor.cc tengwriter.cc tengformatter.cc \
		     tengcontenttype.cc tengerror.cc tengtemplate.cc \
		     tenglex1.cc tengsyntax.yy tengposition.cc \
		     tenglex2impl.ll tengudf.cc tengyystype.cc \
		     tengmd5.cc tengconfiguration.cc tengaux.cc \
		     tenglogging.cc tengsemantic.cc tenghex.cc tengutf8.cc \
		     tengstringview.cc tenglex2.cc tengparserfrag.cc \
		     tengparserdiag.cc

# with these flags (version info etc.)
libteng_la_LDFLAGS = @VERSION_INFO@
libteng_la_LIBADD = $(LTLIBOBJS) ${DEPENS_LIBS}

# explicit rules for proper flex & bison sources compilation
tenglex2.lo: tenglex2impl.cc
tengparsercontext.lo: tengsyntax.cc

# test program
EXTRA_PROGRAMS = teng-example teng-cache test-teng
teng_example_SOURCES = example/example.cc
teng_example_LDADD = libteng.la
teng_cache_SOURCES = example/cache.cc
teng_cache_LDADD = libteng.la

CLEANFILES = @top_builddir@/src/tengsyntax.hh \
	     @top_builddir@/src/tengsyntax.cc \
	     @top_builddir@/src/tenglex2impl.cc \
	     @top_builddir@/src/tenglex2impl.h \
	     @top_builddir@/src/tengdictsyntax.hh \
	     @top_builddir@/src/tengdictsyntax.cc \
	     @top_builddir@/src/tengdictleximpl.h \
	     @top_builddir@/src/tengdictleximpl.cc

doc:
	doxygen

test_teng_LDADD = libteng.la
test_teng_SOURCES = tests/main.cc tests/old.cc tests/fun-escaping.cc \
		    tests/frag.cc tests/vars.cc tests/simple.cc tests/incl.cc \
		    tests/format.cc tests/expr-string.cc tests/expr-int.cc \
		    tests/expr-real.cc tests/expr-other.cc tests/dict.cc \
		    tests/expr-regex.cc tests/ctype.cc tests/cond.cc \
		    tests/expr-tern.cc tests/expr-case.cc tests/fun-date.cc \
		    tests/fun-number.cc tests/fun-string.cc tests/fun-other.cc \
		    tests/queries.cc tests/debug.cc tests/rtvars.cc \
		    tests/builtin-vars.cc

AM_COLOR_TESTS = always
TESTS = test-teng

