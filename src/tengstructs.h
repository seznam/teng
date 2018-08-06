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
 * $Id: tengstructs.h,v 1.6 2007-05-21 15:43:28 vasek Exp $
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
#include <memory>
#include <map>

#include <tengconfig.h>

#include <stdint.h>

namespace Teng {

class FragmentValue_t;
class FragmentList_t;

/**
 * @short Single fragment. Maps names to variables and nested
 *        fragments.
 */
class Fragment_t {
public:
    // types
    using items_t = std::map<std::string, std::unique_ptr<FragmentValue_t>>;
    using const_iterator = items_t::const_iterator;

    // don't copy
    Fragment_t(const Fragment_t &) = delete;
    Fragment_t &operator=(const Fragment_t &) = delete;

    /**
     * @short C'tor.
     */
    Fragment_t() = default;

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
    void addVariable(const std::string &name, IntType_t value);

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
    Fragment_t &addFragment(const std::string &name);

    /**
     * @short Add new nested fragment list.
     * @param name fragment name
     * @return created fragment list
     */
    FragmentList_t &addFragmentList(const std::string &name);

    /**
     * @short Dump fragment to stream.
     * @param o output stream
     */
    void dump(std::ostream &o) const;

    /**
     * @short Dump fragment to stream in json format
     * @param o output stream
     */
    void json(std::ostream &o) const;

    /**
     * @short Returns iterator to fragment item of desired name.
     */
    const_iterator find(const std::string &name) const {
        return items.find(name);
    }

    /**
     * @short Returns iterator to first fragment item.
     */
    const_iterator begin() const {return items.begin();}

    /**
     * @short Returns iterator one past the last fragment item.
     */
    const_iterator end() const {return items.end();}

protected:
    items_t items; //!< fragments data
};

/**
 * @short List of fragments of same name at same level.
 */
class FragmentList_t {
public:
    // types
    using items_t = std::vector<std::unique_ptr<Fragment_t>>;
    using const_iterator = items_t::const_iterator;
    using size_type = items_t::size_type;

    // don't copy
    FragmentList_t(const FragmentList_t &) = delete;
    FragmentList_t &operator=(const FragmentList_t &) = delete;

    /**
     * @short C'tor.
     */
    FragmentList_t() = default;

    /**
     * @short Add given or empty fragment to fragment list.
     * @return created fragment
     */
    Fragment_t &addFragment();

    /**
     * @short Dump fragment list to stream.
     * @param o output stream
     */
    void dump(std::ostream &o) const;

    /**
     * @short Dump fragment to stream in json format
     * @param o output stream
     */
    void json(std::ostream &o) const;

    /**
     * @short Returns the items count.
     */
    size_type size() const {return items.size();}

    /**
     * @short Returns true if list is empty.
     */
    size_type empty() const {return items.empty();}

    /**
     * @short Returns iterator to first fragment item.
     */
    const_iterator begin() const {return items.begin();}

    /**
     * @short Returns iterator one past the last fragment item.
     */
    const_iterator end() const {return items.end();}

    /**
     * @short Returns i-th fragment in the list.
     */
    Fragment_t *operator[](size_type i) const {return items[i].get();}

protected:
    items_t items; //!< the fragment list items
};

/**
 * @short Value in data tree.
 *
 * Shall be scalar (single data) or list of fragments.
 */
class FragmentValue_t {
public:
    // don't copy
    FragmentValue_t(const FragmentValue_t &) = delete;
    FragmentValue_t &operator=(const FragmentValue_t &) = delete;

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
    FragmentValue_t(IntType_t value);

    /**
     * @short Create new scalar value with given value.
     * @param value value of variable
     */
    FragmentValue_t(double value);

    /**
     * @hosrt Create fragment list value.
     * @param fragment_list the list.
     */
    FragmentValue_t(std::unique_ptr<FragmentList_t> fragment_list);

    /**
     * @short Sets value to string.
     */
    void setValue(const std::string &new_value);

    /**
     * @short Sets value to int.
     */
    void setValue(const IntType_t new_value);

    /**
     * @short Sets value to double.
     */
    void setValue(const double new_value);

    /**
     * @hosrt Ensures that value is fragment list and if not then other value
     * is destroyed and new empty fragment list is assigned to value and
     * returned.
     */
    FragmentList_t &ensureFragmentList();

    /**
     * @short Adds new empty fragment to the frament list.
     * @return new fragment
     */
    Fragment_t &addFragment();

    /**
     * @short Dump fragment list to stream.
     * @param o output stream
     */
    void dump(std::ostream &o) const;

    /**
     * @short Dump fragment to stream in json format
     * @param o output stream
     */
    void json(std::ostream &o) const;

    /**
     * @short Returns pointer to nested fragments or nullptr.
     */
    const FragmentList_t *getNestedFragments() const {
        return held_type == type::fragments? nestedFragments.get(): nullptr;
    }

    /**
     * @short Returns pointer to scallar value or nullptr.
     */
    const std::string *getValue() const {
        return held_type != type::fragments? &value: nullptr;
    }

protected:
    /** The type of value.
     */
    enum class type {fragments, integer, floating, string} held_type;

    union {
        /**
         * @short String (scalar) value.
         */
        std::string value;

        /**
         * @short List of nested fragments.
         */
        std::unique_ptr<FragmentList_t> nestedFragments;
    };
};

} // namespace Teng

#endif // TENGSTRUCTS_H

