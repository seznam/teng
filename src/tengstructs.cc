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
 * $Id: tengstructs.cc,v 1.1 2004-07-28 11:36:55 solamyl Exp $
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
 */

#include <stdio.h>
#include <string.h>

#include "tengstructs.h"

using namespace std;

using namespace Teng;

FragmentValue_t::FragmentValue_t()
    : value(), nestedFragments(0)
{}

FragmentValue_t::FragmentValue_t(const string &value)
    : value(value), nestedFragments(0)
{}

FragmentValue_t::FragmentValue_t(long int value)
    : nestedFragments(0)
{
    char buff[100];
    snprintf(buff, sizeof(buff), "%ld", value);
    this->value = string(buff);
}

FragmentValue_t::FragmentValue_t(double _value)
    : nestedFragments(0)
{
    char buff[64];

    // print value to the buffer
    int len = snprintf(buff, sizeof(buff), "%f", _value);
    // find dot in the buffer
    if (!strchr(buff, '.')) {
        // no dot => append ".0"
        value = string(buff, len);
        value.append(".0");
    } else {
        // dot found => find first nonzero character from the right
        for (char *c = buff + len - 1;
             ((*(c - 1) != '.') && (*c == '0'));
             --len, --c);
        // create string
        value = string(buff, len);
    }
}

FragmentValue_t::~FragmentValue_t() {
    delete nestedFragments;
}

Fragment_t::~Fragment_t() {
    for (iterator i = begin(); i != end(); ++i)
        delete i->second;
}

void Fragment_t::addVariable(const string &name,
                                 const string &value)
{
    insert(value_type(name, new FragmentValue_t(value)));
}

void Fragment_t::addVariable(const string &name,
                                 long int value)
{
    insert(value_type(name, new FragmentValue_t(value)));
}

void Fragment_t::addVariable(const string &name,
                                 double value)
{
    insert(value_type(name, new FragmentValue_t(value)));
}

Fragment_t& Fragment_t::addFragment(const string &name) {
    return addFragmentList(name).addFragment();
}

FragmentList_t&
Fragment_t::addFragmentList(const string &name) {
    // find fragment in the list
    iterator i = find(name);
    // if not found insert new value
    if (i == end())
        i = insert(value_type(name,
                              new FragmentValue_t())).first;
    // return nested fragment list
    if (!i->second->nestedFragments)
        i->second->nestedFragments = new FragmentList_t();
    return *i->second->nestedFragments;
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

void FragmentValue_t::dump(ostream &o) const {
    // print value or dump fragment list
    if (nestedFragments) nestedFragments->dump(o);
    else o << '\'' << value << '\'';
}

void FragmentList_t::dump(ostream &o) const {
    o << '[';
    // dump all fragments
    for (const_iterator i = begin(); i != end(); ++i) {
        if (i != begin()) o << ", ";
        (*i)->dump(o);
    }
    o << ']';
}

void Fragment_t::dump(ostream &o) const {
    o << '{';
    // dump all values or fragment list
    for (const_iterator i = begin(); i != end(); ++i) {
        if (i != begin()) o << ", ";
        o << "'" << i->first << "': ";
        i->second->dump(o);
    }
    o << '}';
}
