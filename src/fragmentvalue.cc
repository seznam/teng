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
#include "teng/stringify.h"
#include "teng/fragmentvalue.h"
#include "teng/fragmentlist.h"
#include "teng/fragment.h"

namespace Teng {
namespace {

/** Calls the d'tor.
 */
template <typename type_t>
void dispose(type_t *ptr) {ptr->~type_t();}

} // namespace

FragmentValue_t::FragmentValue_t(FragmentValue_t &&other) noexcept
    : tag_value(other.tag_value)
{
    switch (tag_value) {
    case tag::frag:
        new (&frag_value) Fragment_t(std::move(other.frag_value));
        break;
    case tag::frag_ptr:
        frag_ptr_value = other.frag_ptr_value;
        break;
    case tag::list:
        new (&list_value) FragmentList_t(std::move(other.list_value));
        break;
    case tag::string:
        new (&string_value) std::string(std::move(other.string_value));
        break;
    case tag::integral:
        integral_value = other.integral_value;
        break;
    case tag::real:
        real_value = other.real_value;
        break;
    }
}

FragmentValue_t &FragmentValue_t::operator=(FragmentValue_t &&other) noexcept {
    if (&other != this) {
        if (tag_value == other.tag_value) {
            switch (tag_value) {
            case tag::frag:
                frag_value = std::move(other.frag_value);
                break;
            case tag::frag_ptr:
                frag_ptr_value = other.frag_ptr_value;
                break;
            case tag::list:
                list_value = std::move(other.list_value);
                break;
            case tag::string:
                string_value = std::move(other.string_value);
                break;
            case tag::integral:
                integral_value = other.integral_value;
                break;
            case tag::real:
                real_value = other.real_value;
                break;
            }
        } else {
            this->~FragmentValue_t();
            tag_value = other.tag_value;
            new (this) FragmentValue_t(std::move(other));
        }
    }
    return *this;
}

FragmentValue_t::~FragmentValue_t() noexcept {
    switch (tag_value) {
    case tag::frag:
        dispose(&frag_value);
        break;
    case tag::frag_ptr:
        break;
    case tag::list:
        dispose(&list_value);
        break;
    case tag::string:
        dispose(&string_value);
        break;
    case tag::integral:
        break;
    case tag::real:
        break;
    }
}

void FragmentValue_t::setValue(const std::string &new_value) {
    switch (tag_value) {
    case tag::frag:
        dispose(&frag_value);
        new (&string_value) std::string(new_value);
        tag_value = tag::string;
        break;
    case tag::frag_ptr:
        throw std::runtime_error(__PRETTY_FUNCTION__);
        break;
    case tag::list:
        dispose(&list_value);
        new (&string_value) std::string(new_value);
        tag_value = tag::string;
        break;
    case tag::string:
        string_value = new_value;
        break;
    case tag::integral:
        new (&string_value) std::string(new_value);
        tag_value = tag::string;
        break;
    case tag::real:
        new (&string_value) std::string(new_value);
        tag_value = tag::string;
        break;
    }
}

void FragmentValue_t::setInt(const IntType_t new_value) {
    switch (tag_value) {
    case tag::frag:
        dispose(&frag_value);
        integral_value = new_value;
        tag_value = tag::integral;
        break;
    case tag::frag_ptr:
        throw std::runtime_error(__PRETTY_FUNCTION__);
        break;
    case tag::list:
        dispose(&list_value);
        integral_value = new_value;
        tag_value = tag::integral;
        break;
    case tag::string:
        dispose(&string_value);
        integral_value = new_value;
        tag_value = tag::integral;
        break;
    case tag::integral:
        integral_value = new_value;
        break;
    case tag::real:
        integral_value = new_value;
        tag_value = tag::integral;
        break;
    }
}

void FragmentValue_t::setDouble(double new_value) {
    switch (tag_value) {
    case tag::frag:
        dispose(&frag_value);
        real_value = new_value;
        tag_value = tag::real;
        break;
    case tag::frag_ptr:
        throw std::runtime_error(__PRETTY_FUNCTION__);
        break;
    case tag::list:
        dispose(&list_value);
        real_value = new_value;
        tag_value = tag::real;
        break;
    case tag::string:
        dispose(&string_value);
        real_value = new_value;
        tag_value = tag::real;
        break;
    case tag::integral:
        real_value = new_value;
        tag_value = tag::real;
        break;
    case tag::real:
        real_value = new_value;
        break;
    }
}

Fragment_t &FragmentValue_t::addFragment() {
    return ensureFragmentList().addFragment();
}

std::string FragmentValue_t::toString() const {
    switch (tag_value) {
    case tag::frag:
        return "";
    case tag::frag_ptr:
        return "";
    case tag::list:
        return "";
    case tag::string:
        return string_value;
    case tag::integral:
        return stringify(integral_value);
    case tag::real:
        return stringify(real_value);
    }
    throw std::runtime_error(__PRETTY_FUNCTION__);
}

void FragmentValue_t::json(std::ostream &o) const {
    switch (tag_value) {
    case tag::frag:
        frag_value.json(o);
        break;
    case tag::frag_ptr:
        frag_ptr_value->json(o);
        break;
    case tag::list:
        list_value.json(o);
        break;
    case tag::string:
        json::quote_string(o, string_value);
        break;
    case tag::integral:
        o << integral_value;
        break;
    case tag::real:
        o << real_value;
        break;
    }
}

void FragmentValue_t::dump(std::ostream &o) const {
    switch (tag_value) {
    case tag::frag:
        frag_value.json(o);
        break;
    case tag::frag_ptr:
        frag_ptr_value->json(o);
        break;
    case tag::list:
        list_value.json(o);
        break;
    case tag::string:
        o << '\'' << string_value << '\'';
        break;
    case tag::integral:
        o << '\'' << stringify(integral_value) << '\'';
        break;
    case tag::real:
        o << '\'' << stringify(real_value) << '\'';
        break;
    }
}

FragmentList_t &FragmentValue_t::ensureFragmentList() {
    switch (tag_value) {
    case tag::frag:
        dispose(&frag_value);
        new (&list_value) FragmentList_t();
        tag_value = tag::list;
        break;
    case tag::frag_ptr:
        throw std::runtime_error(__PRETTY_FUNCTION__);
        break;
    case tag::list:
        break;
    case tag::string:
        dispose(&string_value);
        new (&list_value) FragmentList_t();
        tag_value = tag::list;
        break;
    case tag::integral:
        new (&list_value) FragmentList_t();
        tag_value = tag::list;
        break;
    case tag::real:
        new (&list_value) FragmentList_t();
        tag_value = tag::list;
        break;
    }
    return list_value;
}

Fragment_t &FragmentValue_t::ensureFragment() {
    switch (tag_value) {
    case tag::frag:
        break;
    case tag::frag_ptr:
        throw std::runtime_error(__PRETTY_FUNCTION__);
        break;
    case tag::list:
        dispose(&frag_value);
        new (&frag_value) Fragment_t();
        tag_value = tag::frag;
        break;
    case tag::string:
        dispose(&string_value);
        new (&frag_value) Fragment_t();
        tag_value = tag::frag;
        break;
    case tag::integral:
        new (&frag_value) Fragment_t();
        tag_value = tag::frag;
        break;
    case tag::real:
        new (&frag_value) Fragment_t();
        tag_value = tag::frag;
        break;
    }
    return frag_value;
}

} // namespace Teng

