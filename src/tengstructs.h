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
 * $Id: tengstructs.h,v 1.5 2006-01-27 14:03:01 vasek Exp $
 *
 * DESCRIPTION
 * Teng data types.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-17  (vasek)
 *             Created.
*/

#ifndef TENGSTRUCTS_H
#define TENGSTRUCTS_H

#include <string>
#include <vector>
#include <iostream>
#include <map>

namespace Teng {

class FragmentValue_t;
class FragmentList_t;

/**
 * @short Single fragment. Maps names to variables and nested
 *        fragments.
 */
class Fragment_t : private std::map<std::string, FragmentValue_t*> {
public:
    Fragment_t()
        : std::map<std::string, FragmentValue_t*>()
    {}

    ~Fragment_t();

    /**
     * @short Add variable to fragment.
     * @param name variable name
     * @param value variable value
     */
    void addVariable(const std::string &name, const std::string &value);

    /**
     * @short Add variable to fragment.
     * @param name variable name
     * @param value variable value
     */
    void addVariable(const std::string &name, long int value);

    /**
     * @short Add variable to fragment.
     * @param name variable name
     * @param value variable value
     */
    void addVariable(const std::string &name, double value);

    /**
     * @short Add nested fragment.
     * @param name fragment name
     * @return created fragment
     */
    Fragment_t& addFragment(const std::string &name);

    /**
     * @short Add new nested fragment list.
     * @param name fragment name
     * @return created fragment list
     */
    FragmentList_t& addFragmentList(const std::string &name);

    /**
     * @short Dump fragment to stream.
     * @param o output stream
     */
    void dump(std::ostream &o) const;

    using std::map<std::string, FragmentValue_t*>::begin;

    using std::map<std::string, FragmentValue_t*>::end;

    using std::map<std::string, FragmentValue_t*>::find;

    using std::map<std::string, FragmentValue_t*>::const_iterator;

private:
    /**
     * @short Copy constructor intentionally private -- copying
     *        disabled.
     */
    Fragment_t(const Fragment_t&);

    /**
     * @short Assignment operator intentionally private -- assignment
     *        disabled.
     */
    Fragment_t operator=(const Fragment_t&);
};

/**
 * @short List of fragments of same name at same level.
 */
class FragmentList_t : private std::vector<Fragment_t*> {
public:
    inline FragmentList_t()
        : std::vector<Fragment_t*>()
    {}

    /**
     * @short Destroy fragment list.
     */
    ~FragmentList_t();

    /**
     * @short Add given or empty fragment to fragment list.
     * @return created fragment
     */
    Fragment_t& addFragment();

    /**
     * @short Dump fragment list to stream.
     * @param o output stream
     */
    void dump(std::ostream &o) const;

    using std::vector<Fragment_t*>::begin;

    using std::vector<Fragment_t*>::end;

    using std::vector<Fragment_t*>::size;

    using std::vector<Fragment_t*>::empty;

    using std::vector<Fragment_t*>::operator [];

    using std::vector<Fragment_t*>::const_iterator;

private:
    /**
     * @short Copy constructor intentionally private -- copying
     *        disabled.
     */
    FragmentList_t(const FragmentList_t&);
    
    /**
     * @short Assignment operator intentionally private -- assignment
     *        disabled.
     */
    FragmentList_t operator=(const FragmentList_t&);
};

/**
 * @short Value in data tree.
 *
 * Shall be scalar (single data) or list of fragments.
 */
class FragmentValue_t {
public:
    /**
     * @short Create new empty value.
     */
    FragmentValue_t();

    /**
     * @short Destroy value.
     */
    ~FragmentValue_t();

    /**
     * @short Create new scalar value with given value.
     * @param value value of variable
     */
    FragmentValue_t(const std::string &value);

    /**
     * @short Create new scalar value with given value.
     * @param value value of variable
     */
    FragmentValue_t(long int value);

    /**
     * @short Create new scalar value with given value.
     * @param value value of variable
     */
    FragmentValue_t(double value);

    void setValue(const std::string &value);

    void setValue(const long int value);

    void setValue(const double value);

    /**
     * @short Adds new empty fragment to the frament list.
     * @return new fragment
     */
    Fragment_t& addFragment();

    /**
     * @short Dump fragment list to stream.
     * @param o output stream
     */
    void dump(std::ostream &o) const;

    /**
     * @short String (scalar) value.
     * Meaningles if nestedFragments non-null.
     */
    std::string value;

    /**
     * @short List of nested fragments.
     */
    FragmentList_t *nestedFragments;

private:
    /**
     * @short Copy constructor intentionally private -- copying
     *        disabled.
     */
    FragmentValue_t(const FragmentValue_t&);
    
    /**
     * @short Assignment operator intentionally private -- assignment
     *        disabled.
     */
    FragmentValue_t operator=(const FragmentValue_t&);
};

} // namespace Teng

#endif // TENGSTRUCTS_H
