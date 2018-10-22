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
# $Id: Makefile.am,v 1.8 2008-11-14 11:00:02 burlog Exp $
#
# DESCRIPTION
# A makefile template for project Teng.
#
# AUTHORS
# Vaclav Blazek <blazek@firma.seznam.cz>
#
# HISTORY
#       2003-09-26  (vasek)
#                   Created.
#

SUBDIRS = . src tests
# directories temporarily out of order: 'doc'

#
# Removed due to this request:
#
# From: Andrew Betts [mailto:andrew.betts@jasp.com]
#
# Would it be possible for you to release a source tarball without the
# debian/ directory?
#
# From: https://help.ubuntu.com/6.10/ubuntu/packagingguide/C/basic-mistakes.html
# "Q: What do I do with an .orig.tar.gz that already includes a debian/ dir?
#  A: Do not repackage it. You can ask the author(s) to delete the debian/ dir"
#
# debian packaging scripts
#EXTRA_DIST = debian/libteng.control debian/libteng.postinst \
#             debian/libteng-dev.control \
#             debian/make.sh

pkgconfigdir=@libdir@/pkgconfig
pkgconfig_DATA=libteng.pc
