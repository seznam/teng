/*
 * Teng -- a general purpose templating engine.
 * Copyright (C) 2004  Seznam.cz, a.s.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Seznam.cz, a.s.
 * Naskove 1, Praha 5, 15000, Czech Republic
 * http://www.seznam.cz, mailto:teng@firma.seznam.cz
 *
 *
 * $Id: tengstructs.cc,v 1.4 2007-05-21 15:43:28 vasek Exp $
 *
 * DESCRIPTION
 * Teng data types -- implementation.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-30  (vasek)
 *             Created.
 * 2005-06-21  (roman)
 *             Win32 support.
*/

#include "jsonutils.h"
#include "teng/fragmentvalue.h"
#include "teng/fragmentlist.h"
#include "teng/fragment.h"

namespace Teng {
namespace {

template <typename where_t, typename type_t>
void replace_item(where_t &&items, const std::string &name, type_t &&value) {
    auto iitem = items.find(name);
    if (iitem != items.end())
        return iitem->second.setValue(std::forward<type_t>(value));
    items.emplace_hint(iitem, name, std::forward<type_t>(value));
}

} // namespace

void Fragment_t::json(std::ostream &o) const {
    o << '{';
   for (auto ivalue = begin(), evalue = end(); ivalue != evalue; ++ivalue) {
        if (ivalue != begin()) o << ", ";
        json::quote_string(o, ivalue->first);
        o << ": ";
        ivalue->second.json(o);
    }
    o << '}';
}

void Fragment_t::dump(std::ostream &o) const {
    o << '{';
    for (auto ivalue = begin(), evalue = end(); ivalue != evalue; ++ivalue) {
        if (ivalue != begin()) o << ", ";
        o << "'" << ivalue->first << "': ";
        ivalue->second.dump(o);
    }
    o << '}';
}

void
Fragment_t::addVariable(const std::string &name, const std::string &value) {
    replace_item(items, name, value);
}

void Fragment_t::addIntVariable(const std::string &name, IntType_t value) {
    replace_item(items, name, value);
}

void Fragment_t::addRealVariable(const std::string &name, double value) {
    replace_item(items, name, value);
}

Fragment_t &Fragment_t::addFragment(const std::string &name) {
    return addFragmentList(name).addFragment();
}

FragmentList_t &
Fragment_t::addFragmentList(const std::string &name) {
    auto iitem = items.find(name);
    if (iitem != items.end())
        return iitem->second.ensureFragmentList();
    auto create_list = TypeTag_t<FragmentList_t>();
    return items.emplace_hint(iitem, name, create_list)->second.list_value;
}

void Fragment_t::addValue(const std::string &name, Fragment_t &&value) {
    addValue(name, FragmentValue_t(std::move(value)));
}

void Fragment_t::addValue(const std::string &name, FragmentList_t &&value) {
    addValue(name, FragmentValue_t(std::move(value)));
}

void Fragment_t::addValue(const std::string &name, FragmentValue_t &&value) {
    auto iitem = items.find(name);
    if (iitem != items.end())
        iitem->second = std::move(value);
    else items.emplace_hint(iitem, name, std::move(value));
}

} // namespace Teng

