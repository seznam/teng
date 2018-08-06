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
        o << ": ";
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
        return iitem->second->ensureFragmentList();
    return items.emplace_hint(iitem, name, std::make_unique<FragmentValue_t>())
           ->second->ensureFragmentList();
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
    : tag_value(tag::fragments), nestedFragments()
{}

FragmentValue_t::FragmentValue_t(const std::string &value)
    : tag_value(tag::string), string_value(value)
{}

FragmentValue_t::FragmentValue_t(IntType_t value)
    : tag_value(tag::integral), integral_value(value)
{}

FragmentValue_t::FragmentValue_t(double value)
    : tag_value(tag::real), real_value(value)
{}

FragmentValue_t::~FragmentValue_t() {
    switch (tag_value) {
    case tag::fragments: dispose(&nestedFragments); break;
    case tag::string: dispose(&string_value); break;
    case tag::integral: break;
    case tag::real: break;
    }
}

void FragmentValue_t::setValue(const std::string &new_value) {
    switch (tag_value) {
    case tag::fragments:
        dispose(&nestedFragments);
        new (&string_value) std::string(new_value);
        break;
    case tag::string:
        string_value = new_value;
        break;
    case tag::integral:
        new (&string_value) std::string(new_value);
        break;
    case tag::real:
        new (&string_value) std::string(new_value);
        break;
    }
    tag_value = tag::string;
}

void FragmentValue_t::setInt(const IntType_t new_value) {
    switch (tag_value) {
    case tag::fragments:
        dispose(&nestedFragments);
        integral_value = new_value;
        break;
    case tag::string:
        dispose(&string_value);
        integral_value = new_value;
        break;
    case tag::integral:
        integral_value = new_value;
        break;
    case tag::real:
        integral_value = new_value;
        break;
    }
    tag_value = tag::integral;
}

void FragmentValue_t::setDouble(double new_value) {
    switch (tag_value) {
    case tag::fragments:
        dispose(&nestedFragments);
        real_value = new_value;
        break;
    case tag::string:
        dispose(&string_value);
        real_value = new_value;
        break;
    case tag::integral:
        real_value = new_value;
        break;
    case tag::real:
        real_value = new_value;
        break;
    }
    tag_value = tag::real;
}

FragmentList_t &FragmentValue_t::ensureFragmentList() {
    switch (tag_value) {
    case tag::fragments:
        break;
    case tag::string:
        dispose(&string_value);
        new (&nestedFragments) FragmentList_t();
        tag_value = tag::fragments;
        break;
    case tag::integral:
        new (&nestedFragments) FragmentList_t();
        tag_value = tag::fragments;
        break;
    case tag::real:
        new (&nestedFragments) FragmentList_t();
        tag_value = tag::fragments;
        break;
    }
    return nestedFragments;
}

Fragment_t &FragmentValue_t::addFragment() {
    return ensureFragmentList().addFragment();
}

std::string FragmentValue_t::getValue() const {
    switch (tag_value) {
    case tag::fragments: return "";
    case tag::string: return string_value;
    case tag::integral: return stringify(integral_value);
    case tag::real: return stringify(real_value);
    }
}

void FragmentValue_t::json(std::ostream &o) const {
    switch (tag_value) {
    case tag::fragments: nestedFragments.json(o); break;
    case tag::string: quote_json_string(o, string_value); break;
    case tag::integral: o << integral_value; break;
    case tag::real: o << real_value; break;
    }
}

void FragmentValue_t::dump(std::ostream &o) const {
    switch (tag_value) {
    case tag::fragments: nestedFragments.json(o); break;
    case tag::string: o << '\'' << string_value << '\''; break;
    case tag::integral: o << '\'' << stringify(integral_value) << '\''; break;
    case tag::real: o << '\'' << stringify(real_value) << '\''; break;
    }
}

} // namespace Teng

