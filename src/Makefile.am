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


# warn on all
AM_CXXFLAGS = -Wall

# extra compiler options
AM_CPPFLAGS = -D_ISOC99_SOURCE -fPIC ${pcrecpp_CFLAGS}

# these are generated and thus ought to be prepared before anything else
BUILT_SOURCES = tenglex2.cc tengsyntax.cc

# install these headers
include_HEADERS = teng.h tengstructs.h tengwriter.h tengerror.h tengconfig.h tengudf.h

noinst_HEADERS = tengcache.h tengcode.h tengcontenttype.h \
                 tengdictionary.h tengformatter.h tengfragmentstack.h \
                 tengfunction.h tenginstruction.h tenglex1.h \
                 tengparsercontext.h tengparservalue.h tengprocessor.h \
                 tengprogram.h tengsourcelist.h tengtemplate.h \
                 tengutil.h tengwriter.h tengsyntax.h tengconfiguration.h \
		 tengplatform.h tengaux.h

# compile this library
lib_LTLIBRARIES = libteng.la

# from the these sources
libteng_la_SOURCES = teng.cc tengstructs.cc tengutil.cc tengcache.cc \
                     tengsourcelist.cc tengdictionary.cc tenginstruction.cc \
                     tengparservalue.cc tengparsercontext.cc tengprogram.cc \
                     tengfunction.cc tengprocessor.cc tengwriter.cc \
                     tengformatter.cc tengcontenttype.cc \
                     tengtemplate.cc tenglex1.cc tengsyntax.yy \
                     tenglex2.ll tengcode.cc tengudf.cc \
                     tengmd5.cc tengconfiguration.cc tengaux.cc

# with these flags (version info etc.)
libteng_la_LDFLAGS = @VERSION_INFO@
libteng_la_LIBADD = $(LTLIBOBJS) ${pcrecpp_LIBS}

# explicit rules for proper flex & bison sources compilation
tenglex2.cc: tenglex2.ll tengsyntax.h tengsyntax.cc
tengsyntax.h: tengsyntax.yy
tengsyntax.cc: tengsyntax.yy

# test program
#noinst_PROGRAMS = 

doc:
	doxygen
