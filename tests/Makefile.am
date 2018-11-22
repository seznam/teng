#
# Teng -- a general purpose templating engine.
# Copyright (C) 2018  Seznam.cz, a.s.
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

AM_LDFLAGS  = -L@top_builddir@/src $(BOOST_LDFLAGS)
AM_CPPFLAGS = -std=c++14 -I$(top_srcdir)/src $(BOOST_CPPFLAGS) -DBOOST_TEST_DYN_LINK
LDADD       = -lteng ${BOOST_SYSTEM_LIB} ${BOOST_FILESYSTEM_LIB} ${BOOST_UNIT_TEST_FRAMEWORK_LIB}


# test programs
check_PROGRAMS =

test_base_SOURCES = test_base.cc
test_filesystem_SOURCES = test_filesystem.cc

if CPP14
    check_PROGRAMS += test_base test_filesystem
else
	check_PROGRAMS +=
endif !CPP14

AM_COLOR_TESTS = always
TESTS = $(check_PROGRAMS)
