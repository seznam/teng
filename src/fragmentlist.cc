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
#include "teng/config.h"
#include "teng/fragmentvalue.h"
#include "teng/fragmentlist.h"
#include "teng/fragment.h"

namespace Teng {

Fragment_t &FragmentList_t::addFragment() {
    items.emplace_back(TypeTag_t<Fragment_t>());
    return items.back().frag_value;
}

FragmentList_t &FragmentList_t::addFragmentList() {
    items.emplace_back(TypeTag_t<FragmentList_t>());
    return items.back().list_value;
}

void FragmentList_t::addValue(const std::string &value) {
    items.emplace_back(value);
}

void FragmentList_t::addIntValue(IntType_t value) {
    items.emplace_back(value);
}

void FragmentList_t::addRealValue(double value) {
    items.emplace_back(value);
}

void FragmentList_t::addValue(Fragment_t &&value) {
    items.emplace_back(std::move(value));
}

void FragmentList_t::addValue(FragmentList_t &&value) {
    items.emplace_back(std::move(value));
}

void FragmentList_t::addValue(FragmentValue_t &&value) {
    items.emplace_back(std::move(value));
}

void FragmentList_t::json(std::ostream &o) const {
    o << '[';
    for (auto ifrag = begin(), efrag = end(); ifrag != efrag; ++ifrag) {
        if (ifrag != begin()) o << ", ";
        ifrag->json(o);
    }
    o << ']';
}

void FragmentList_t::dump(std::ostream &o) const {
    o << '[';
    for (auto ifrag = begin(), efrag = end(); ifrag != efrag; ++ifrag) {
        if (ifrag != begin()) o << ", ";
        ifrag->dump(o);
    }
    o << ']';
}

} // namespace Teng

