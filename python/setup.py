#!/usr/bin/env python
# -*- mode: Python -*-
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
# $Id: setup.py,v 1.3 2006-07-11 14:46:20 sten__ Exp $
#
# DESCRIPTION
# Build script for python teng module.
#
# AUTHORS
# Vaclav Blazek <blazek@firma.seznam.cz>
#
# HISTORY
# 2003-09-26  (vasek)
#             Created.
#
# 2004-05-03  (vasek)
#             Modified for new packaging.
#
# USAGE
# python ./setup.py --help
#


# This version is used when packaging.
VERSION          = "1.0.13"

# Maintainer of this module.
MAINTAINER       = "Vaclav Blazek"
MAINTAINER_EMAIL = "blazek@firma.seznam.cz"

# Descriptions
DESCRIPTION      = "Teng -- general purpose templating system"
LONG_DESCRIPTION = "Teng is a powerful and easy to use templating system.\n"

# You probably don't need to edit anything below this line

from distutils.core import setup, Extension

### HACK: force g++ as compiler and linker.
### Credits go to Eric Jones <eric at enthought.com>
from distutils.unixccompiler import UnixCCompiler
import distutils.sysconfig
old_init_posix = distutils.sysconfig._init_posix
def _init_posix():
    old_init_posix()
    distutils.sysconfig._config_vars['LDSHARED'] = 'g++ -shared'
    distutils.sysconfig._config_vars['CC'] = 'g++'
#enddef
distutils.sysconfig._init_posix = _init_posix

# Main core
setup (
    name             = "teng",
    version          = VERSION,
    author           = "Vaclav Blazek",
    author_email     = "blazek@firma.seznam.cz",
    maintainer       = MAINTAINER,
    maintainer_email = MAINTAINER_EMAIL,
    description      = DESCRIPTION,
    long_description = LONG_DESCRIPTION,
    ext_modules = [
        Extension ("tengmodule", ["tengmodule.cc"], libraries=["teng"])
    ]
)
