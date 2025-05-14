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
# Gdb pretty printes.
#
# AUTHORS
# Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
#
# HISTORY
#       2018-11-28  (burlog)
#                   Created.
#

import gdb
import itertools

_use_gdb_pp = True
try:
    import gdb.printing
except ImportError:
    _use_gdb_pp = False

class TengFragmentValuePrinter:
    def __init__(self, typename, val):
        self.val = val.cast(gdb.lookup_type(typename))
        self.typename = typename

    def to_string(self):
        tag_value = int(self.val["tag_value"])
        if tag_value == 0:
            ref = self.val["frag_value"]
            return "%s(frag_value=%s)" % (self.typename, ref.address)
        elif tag_value == 1:
            ref = self.val["frag_ptr_value"]
            return "%s(frag_ptr_value=%s)" % (self.typename, ref)
        elif tag_value == 2:
            ref = self.val["list_value"]
            start = ref["items"]['_M_impl']['_M_start']
            finish = ref["items"]['_M_impl']['_M_finish']
            size = int(finish - start)
            return "%s(list_value=%s, len=%s)" % (self.typename, ref.address, size)
        elif tag_value == 3:
            return "%s(%s)" % (self.typename, self.val["integral_value"])
        elif tag_value == 4:
            return "%s(%s)" % (self.typename, self.val["real_value"])
        elif tag_value == 5:
            return "%s(%s)" % (self.typename, self.val["string_value"])
        return "%s(unknown_frag_value_of_tag=%s)" % (self.typename, tag_value)

class TengValuePrinter:
    def __init__(self, typename, val):
        self.val = val.cast(gdb.lookup_type(typename))
        self.typename = typename

    def to_string(self):
        tag_value = int(self.val["tag_value"])
        if tag_value == 0:
            return "%s(undefined)" % (self.typename)
        elif tag_value == 1:
            return "%s(\"%s\")" % (self.typename, self.val["string_value"])
        elif tag_value == 2:
            ref = self.val["string_ref_value"]
            return "%s(@\"%s\")" % (self.typename, ref["ptr"].string(length=ref["len"]))
        elif tag_value == 3:
            return "%s(%s)" % (self.typename, self.val["integral_value"])
        elif tag_value == 4:
            return "%s(%s)" % (self.typename, self.val["real_value"])
        elif tag_value == 5:
            ref = self.val["frag_ref_value"]
            return "%s(frag_ref_value.ptr=%s)" % (self.typename, ref["ptr"])
        elif tag_value == 6:
            ref = self.val["list_ref_value"]
            return "%s(list_ref_value.ptr=%s, list_ref_value.i=%s)" % (self.typename, ref["ptr"], ref["i"])
        elif tag_value == 7:
            return "%s(%s)" % (self.typename, self.val["regex_value"])
        return "%s(unknown_value_of_tag=%s)" % (self.typename, tag_value)

class TengStringView:
    def __init__(self, typename, val):
        self.val = val.cast(gdb.lookup_type(typename))
        self.typename = typename

    def to_string(self):
        ptr = self.val["ptr"]
        size = self.val["len"]
        return "%s(str=%s, len=%s)" % (self.typename, ptr.string(length=size), size)

class RxPrinter(object):
    def __init__(self, name, function):
        super(RxPrinter, self).__init__()
        self.name = name
        self.function = function
        self.enabled = True

    def invoke(self, value):
        if not self.enabled:
            return None
        return self.function(self.name, value)

class Printer(object):
    def __init__(self, name):
        super(Printer, self).__init__()
        self.name = name
        self.subprinters = []
        self.lookup = {}
        self.enabled = True

    def add(self, name, function):
        printer = RxPrinter(name, function)
        self.subprinters.append(printer)
        self.lookup[name] = printer

    @staticmethod
    def get_basic_type(type):
        if type.code == gdb.TYPE_CODE_REF:
            type = type.target()
        type = type.unqualified().strip_typedefs()
        return type.tag

    def __call__(self, val):
        typename = self.get_basic_type(val.type)
        if not typename:
            return None
        if typename in self.lookup:
            return self.lookup[typename].invoke(val)
        return None

teng_printer = None

def register_teng_printers(obj):
    global _use_gdb_pp
    global teng_printer

    if _use_gdb_pp:
        gdb.printing.register_pretty_printer(obj, teng_printer)
    else:
        if obj is None:
            obj = gdb
        obj.pretty_printers.append(teng_printer)

def build_teng_dictionary():
    global teng_printer

    teng_printer = Printer("teng")
    teng_printer.add('Teng::Value_t', TengValuePrinter)
    teng_printer.add('Teng::FragmentValue_t', TengFragmentValuePrinter)
    teng_printer.add('Teng::string_view_t', TengStringView)

build_teng_dictionary()

