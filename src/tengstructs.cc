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

#include <cstdio>
#include <cstring>
#include <sstream>
#include <iomanip>

#include "tengstructs.h"
#include "tengplatform.h"

namespace Teng {
namespace {

template <typename type_t>
void dispose(type_t *ptr) {ptr->~type_t();}

std::string stringify(IntType_t value) {
    std::ostringstream os;
    os << value;
    return os.str();
}

std::string stringify(double value) {
    // produce at least d.d
    char buffer[64];
    auto len = snprintf(buffer, sizeof(buffer), "%#f", value);

    // remove trialing zeroes
    for (; len > 2; --len) {
        if (buffer[len - 1] == '0')
            if (buffer[len - 2] != '.')
                continue;
        break;
    }

    // done
    return std::string(buffer, len);
}

template <typename where_t, typename type_t>
void replace_item(where_t &&items, const std::string &name, type_t &&value) {
    auto iitem = items.find(name);
    if (iitem != items.end())
        return iitem->second->setValue(value);
    items.emplace_hint(iitem, name, std::make_unique<FragmentValue_t>(value));
}

void unicode_char(std::ostream &o, char ch) {
    o << "\\u00" << std::hex << std::setw(2) << std::setfill('0') << int(ch);
}

void quote_json_string(std::ostream &o, const std::string &value) {
    o << '"';
    for (char ch: value) {
        switch (ch) {
        case 0 ... 8:
        case 11 ... 12:
        case 14 ... 31: unicode_char(o, ch); break;
        case '\n': o << "\\n"; break;
        case '\r': o << "\\r"; break;
        case '\t': o << "\\t"; break;
        case '\\': o << "\\\\"; break;
        case '"': o << "\\\""; break;
        default: o << ch; break;
        }
    }
    o << '"';
}

} // namespace

Fragment_t &FragmentList_t::addFragment() {
    items.push_back(std::make_unique<Fragment_t>());
    return *items.back();
}

void Fragment_t::json(std::ostream &o) const {
    o << '{';
   for (const_iterator i = begin(); i != end(); ++i) {
        if (i != begin()) o << ", ";
        quote_json_string(o, i->first);
        o << " : ";
        i->second->json(o);
    }
    o << '}';
}

void Fragment_t::dump(std::ostream &o) const {
    o << '{';
    for (const_iterator i = begin(); i != end(); ++i) {
        if (i != begin()) o << ", ";
        o << "'" << i->first << "': ";
        i->second->dump(o);
    }
    o << '}';
}

void
Fragment_t::addVariable(const std::string &name, const std::string &value) {
    replace_item(items, name, value);
}

void Fragment_t::addVariable(const std::string &name, IntType_t value) {
    replace_item(items, name, value);
}

void Fragment_t::addVariable(const std::string &name, double value) {
    replace_item(items, name, value);
}

Fragment_t &Fragment_t::addFragment(const std::string &name) {
    return addFragmentList(name).addFragment();
}

FragmentList_t &
Fragment_t::addFragmentList(const std::string &name) {
    auto iitem = items.find(name);
    if (iitem != items.end())
        return iitem->second->ensureFragmentList();
    return items.emplace_hint(
        iitem,
        name,
        std::make_unique<FragmentValue_t>(std::make_unique<FragmentList_t>())
    )->second->ensureFragmentList();
}

void FragmentList_t::json(std::ostream &o) const {
    o << '[';
    for (const_iterator i = begin(); i != end(); ++i) {
        if (i != begin()) o << ", ";
        (*i)->json(o);
    }
    o << ']';
}

void FragmentList_t::dump(std::ostream &o) const {
    o << '[';
    for (const_iterator i = begin(); i != end(); ++i) {
        if (i != begin()) o << ", ";
        (*i)->dump(o);
    }
    o << ']';
}

FragmentValue_t::FragmentValue_t()
    : held_type(type::string), value()
{}

FragmentValue_t::FragmentValue_t(const std::string &value)
    : held_type(type::string), value(value)
{}

FragmentValue_t::FragmentValue_t(IntType_t value)
    : held_type(type::integer), value(stringify(value))
{}

FragmentValue_t::FragmentValue_t(double value)
    : held_type(type::floating), value(stringify(value))
{}

FragmentValue_t::FragmentValue_t(std::unique_ptr<FragmentList_t> fragment_list)
    : held_type(type::fragments), nestedFragments(std::move(fragment_list))
{}

FragmentValue_t::~FragmentValue_t() {
    switch (held_type) {
    case type::fragments: dispose(&nestedFragments); break;
    case type::string: dispose(&value); break;
    case type::integer: dispose(&value); break;
    case type::floating: dispose(&value); break;
    }
}

void FragmentValue_t::setValue(const std::string &new_value) {
    if (held_type == type::fragments) {
        dispose(&nestedFragments);
        new (&value) std::string(new_value);
    } else value = new_value;
    held_type = type::string;
}

void FragmentValue_t::setValue(const IntType_t new_value) {
    if (held_type == type::fragments) {
        dispose(&nestedFragments);
        new (&value) std::string(stringify(new_value));
    } else value = stringify(new_value);;
    held_type = type::integer;
}

void FragmentValue_t::setValue(const double new_value) {
    if (held_type == type::fragments) {
        dispose(&nestedFragments);
        new (&value) std::string(stringify(new_value));
    } else value = stringify(new_value);
    held_type = type::floating;
}

FragmentList_t &FragmentValue_t::ensureFragmentList() {
    if (held_type != type::fragments) {
        dispose(&value);
        new (&nestedFragments)
            std::unique_ptr<FragmentList_t>(new FragmentList_t());
    }
    return *nestedFragments;
}

Fragment_t &FragmentValue_t::addFragment() {
    ensureFragmentList().addFragment();
    return nestedFragments->addFragment();
}

void FragmentValue_t::json(std::ostream &o) const {
    switch (held_type) {
    case type::fragments: nestedFragments->json(o); break;
    case type::string: quote_json_string(o, value); break;
    case type::integer: o << value; break;
    case type::floating: o << value; break;
    }
}

void FragmentValue_t::dump(std::ostream &o) const {
    if (held_type == type::fragments) nestedFragments->dump(o);
    else o << '\'' << value << '\'';
}

} // namespace Teng

