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

#include "tengconfig.h"
#include "tengjsonutils.h"
#include "tengfragmentvalue.h"
#include "tengfragmentlist.h"
#include "tengfragment.h"

namespace Teng {

Fragment_t &FragmentList_t::addFragment() {
    items.emplace_back(TypeTag_t<Fragment_t>());
    return items.back().frag_value;
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

