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
 * $Id: tengparservalue.cc,v 1.4 2008-11-14 11:00:04 burlog Exp $
 *
 * DESCRIPTION
 * Teng parser value data type.
 *
 * AUTHORS
 * Jan Nemec <jan.nemec@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-22  (jan)
 *             Created.
 * 2005-06-21  (roman)
 *             Win32 support.
 * 2018-06-07  (burlog)
 *             Make it more type safe.
 */

#include <ostream>
#include <sstream>

#include "tengjsonutils.h"
#include "tengstructs.h"
#include "tengplatform.h"
#include "tengformatter.h"
#include "tengregex.h"
#include "tengvalue.h"

namespace Teng {

std::ostream &operator<<(std::ostream &os, const Regex_t &regex) {
    os << '/' << regex.pattern() << '/';
    if (regex.flags()->ignore_case) os << 'i';
    if (regex.flags()->global) os << 'g';
    if (regex.flags()->multiline) os << 'm';
    if (regex.flags()->anchored) os << 'A';
    if (regex.flags()->dollar_endonly) os << 'D';
    if (regex.flags()->extended) os << 'e';
    if (regex.flags()->extra) os << 'X';
    if (regex.flags()->ungreedy) os << 'U';
    return os;
}

Value_t::Value_t(const FragmentValue_t *value) noexcept {
    switch (value->type()) {
    case FragmentValue_t::tag::frags:
        new (this) Value_t(&value->frags_value);
        break;
    case FragmentValue_t::tag::frag_ptr:
        new (this) Value_t(value->ptr_value);
        break;
    case FragmentValue_t::tag::frag:
        new (this) Value_t(&value->frag_value);
        break;
    case FragmentValue_t::tag::string:
        // saves some allocation, app data lives longer than value
        new (this) Value_t(string_view_t(value->string_value));
        break;
    case FragmentValue_t::tag::integral:
        new (this) Value_t(value->integral_value);
        break;
    case FragmentValue_t::tag::real:
        new (this) Value_t(value->real_value);
        break;
    }
}

void Value_t::json(std::ostream &out) const {
    switch (tag_value) {
    case Value_t::tag::undefined:
        out << "null";
        break;
    case Value_t::tag::integral:
        out << integral_value;
        break;
    case Value_t::tag::real:
        out << real_value;
        break;
    case Value_t::tag::string:
        json::quote_string(out, string_value);
        break;
    case Value_t::tag::string_ref:
        json::quote_string(out, string_ref_value);
        break;
    case Value_t::tag::frag_ref:
        frag_ref_value.ptr->json(out);
        break;
    case Value_t::tag::list_ref:
        list_ref_value.ptr->json(out);
        break;
    case Value_t::tag::regex:
        out << "null";
        break;
    }
}

std::ostream &operator<<(std::ostream &out, const Value_t &v) {
    switch (v.tag_value) {
    case Value_t::tag::undefined:
        out << "undefined";
        break;
    case Value_t::tag::integral:
        out << "integral(" << v.integral_value << ')';
        break;
    case Value_t::tag::real:
        out << "real(" << v.real_value << ')';
        break;
    case Value_t::tag::string:
        out << "string(" << v.string_value << ')';
        break;
    case Value_t::tag::string_ref:
        out << "string_ref(" << v.string_ref_value << ')';
        break;
    case Value_t::tag::frag_ref:
        out << "frag_ref(@" << v.frag_ref_value.ptr << ')';
        break;
    case Value_t::tag::list_ref:
        out << "list_ref(@" << v.list_ref_value.ptr
            << ',' << v.list_ref_value.i
            << ',' << v.list_ref_value.ptr->size() << ')';
        break;
    case Value_t::tag::regex:
        out << "regex(" << *v.regex_value << ')';
        break;
    }
    return out;
}

void Value_t::dispose_regex() {regex_value.~regex_type();}

void Value_t::assign_regex(const regex_type &value) {regex_value = value;}

void Value_t::assign_regex(regex_type &&value) {regex_value = std::move(value);}

} // namespace Teng

