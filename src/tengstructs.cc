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

#include "tengstructs.h"
#include "tengplatform.h"

namespace Teng {

FragmentValue_t::FragmentValue_t()
    : value(), nestedFragments(0)
{}

FragmentValue_t::FragmentValue_t(const std::string &value)
    : value(value), nestedFragments(0)
{}

FragmentValue_t::FragmentValue_t(IntType_t value_)
    : nestedFragments(0)
{
    setValue(value_);
}

FragmentValue_t::FragmentValue_t(double value_)
    : nestedFragments(0)
{
    setValue(value_);
}

void FragmentValue_t::setValue(const std::string &value_) {
    // get rid of nested fragments if exist
    if (nestedFragments) {
        delete nestedFragments;
        nestedFragments = 0;
    }

    value = value_;
}

void FragmentValue_t::setValue(const IntType_t value_) {
    // get rid of nested fragments if exist
    if (nestedFragments) {
        delete nestedFragments;
        nestedFragments = 0;
    }

    std::ostringstream os;
    os << value_;
    value = os.str();
}

void FragmentValue_t::setValue(const double value_) {
    // get rid of nested fragments if exist
    if (nestedFragments) {
        delete nestedFragments;
        nestedFragments = 0;
    }

    char buff[64];

    // print value to the buffer
    int len = snprintf(buff, sizeof(buff), "%f", value_);
    // find dot in the buffer
    if (!strchr(buff, '.')) {
        // no dot => append ".0"
        value = std::string(buff, len);
        value.append(".0");
    } else {
        // dot found => find first nonzero character from the right
        for (char *c = buff + len - 1;
             ((*(c - 1) != '.') && (*c == '0'));
             --len, --c);
        // create string
        value = std::string(buff, len);
    }
}

FragmentValue_t::~FragmentValue_t() {
    delete nestedFragments;
}

Fragment_t::~Fragment_t() {
    for (iterator i = begin(); i != end(); ++i)
        delete i->second;
}

void Fragment_t::addVariable(const std::string &name, const std::string &value) {
    // insert dummy zero
    std::pair<iterator, bool> inserted(insert(value_type(name, 0)));

    FragmentValue_t *&v = inserted.first->second;

    if (inserted.second) {
        // succeeded, replace by value
        v = new FragmentValue_t(value);
    } else {
         // already present => destroy and create new
        v->setValue(value);
    }
}

void Fragment_t::addVariable(const std::string &name, IntType_t value) {
    // insert dummy zero
    std::pair<iterator, bool> inserted(insert(value_type(name, 0)));

    FragmentValue_t *&v = inserted.first->second;

    if (inserted.second) {
        // succeeded, replace by value
        v = new FragmentValue_t(value);
    } else {
        // already present => destroy and create new
        v->setValue(value);
    }
}

void Fragment_t::addVariable(const std::string &name, double value) {
    // insert dummy zero
    std::pair<iterator, bool> inserted(insert(value_type(name, 0)));

    FragmentValue_t *&v = inserted.first->second;

    if (inserted.second) {
        // succeeded, replace by value
        v = new FragmentValue_t(value);
    } else {
         // already present => destroy and create new
        v->setValue(value);
    }
}

Fragment_t& Fragment_t::addFragment(const std::string &name) {
    return addFragmentList(name).addFragment();
}

FragmentList_t&
Fragment_t::addFragmentList(const std::string &name) {
    // insert dummy zero
    std::pair<iterator, bool> inserted(insert(value_type(name, 0)));

    FragmentValue_t *&v = inserted.first->second;

    if (inserted.second) {
        // succeeded, replace by new value with an empty fragment list
        v = new FragmentValue_t();
        v->nestedFragments = new FragmentList_t();
    } else {
        // already present

        // get rid of scalar value and create an empty fragment list if scalar
        if (!v->nestedFragments) {
            v->value.erase();
            v->nestedFragments = new FragmentList_t();
        }
    }

    // return fragment list
    return *v->nestedFragments;
}

Fragment_t& FragmentValue_t::addFragment() {
    if (!nestedFragments) nestedFragments = new FragmentList_t();

    return nestedFragments->addFragment();
}

FragmentList_t::~FragmentList_t() {
    for (iterator i = begin(); i != end(); ++i)
        delete *i;
}

Fragment_t& FragmentList_t::addFragment() {
    // add new (empty) fragment
    push_back(new Fragment_t());
    // return it
    return *back();
}

void FragmentValue_t::json(std::ostream &o) const {
    // print value or dump fragment list
    if (nestedFragments) nestedFragments->json(o);
    else o << '"' << value << '"';
}


void FragmentValue_t::dump(std::ostream &o) const {
    // print value or dump fragment list
    if (nestedFragments) nestedFragments->dump(o);
    else o << '\'' << value << '\'';
}

void FragmentList_t::json(std::ostream &o) const {
    o << '[';
    // dump all fragments
    for (const_iterator i = begin(); i != end(); ++i) {
        if (i != begin()) o << ", ";
        (*i)->json(o);
    }
    o << ']';
}

void FragmentList_t::dump(std::ostream &o) const {
    o << '[';
    // dump all fragments
    for (const_iterator i = begin(); i != end(); ++i) {
        if (i != begin()) o << ", ";
        (*i)->dump(o);
    }
    o << ']';
}


void Fragment_t::json(std::ostream &o) const {
    o << '{';
    // dump all values or fragment list
    for (const_iterator i = begin(); i != end(); ++i) {
        if (i != begin()) o << ", ";
        o << "\"" << i->first << "\" : ";
        i->second->json(o);
    }
    o << '}';
}

void Fragment_t::dump(std::ostream &o) const {
    o << '{';
    // dump all values or fragment list
    for (const_iterator i = begin(); i != end(); ++i) {
        if (i != begin()) o << ", ";
        o << "'" << i->first << "': ";
        i->second->dump(o);
    }
    o << '}';
}

} // namespace Teng

