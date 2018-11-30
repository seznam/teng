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

#ifndef TENGFRAGMENTLIST_H
#define TENGFRAGMENTLIST_H

#include <string>
#include <vector>
#include <cstdint>

#include <teng/config.h>

namespace Teng {

// forwards
class Fragment_t;
class FragmentValue_t;
class FragmentList_t;

/**
 * @short List of fragment values of same name at same level.
 */
class FragmentList_t {
public:
    // types
    using Items_t = std::vector<FragmentValue_t>;
    using size_type = Items_t::size_type;
    using const_iterator = Items_t::const_iterator;
    using iterator = Items_t::iterator;

    // don't copy
    FragmentList_t(const FragmentList_t &) = delete;
    FragmentList_t &operator=(const FragmentList_t &) = delete;

    /** C'tor.
     */
     FragmentList_t() noexcept = default;

    /**
     * @short C'tor: move.
     */
    FragmentList_t(FragmentList_t &&other) noexcept = default;

    /**
     * @short Assigment: move.
     */
    FragmentList_t &operator=(FragmentList_t &&other) noexcept = default;

    /** D'tor.
     */
    ~FragmentList_t() noexcept = default;

    /**
     * @short Add empty fragment to fragment list.
     * @return created fragment
     */
    Fragment_t &addFragment();

    /**
     * @short Add fragment list to fragment list.
     * @return created fragment
     */
    FragmentList_t &addFragmentList();

    /**
     * @short Add value to list..
     * @param value variable value
     */
    void addValue(const std::string &value);

    /**
     * @short Add value to list..
     * @param value variable value
     */
    template <
        typename type_t,
        std::enable_if_t<std::is_integral<type_t>::value, bool> = true
    > void addValue(type_t value) {
        addIntVariable(value);
    }

    /**
     * @short Add value to list..
     * @param value variable value
     */
    template <
        typename type_t,
        std::enable_if_t<std::is_floating_point<type_t>::value, bool> = true
    > void addValue(type_t value) {
        addRealVariable(value);
    }

    /**
     * @short Add value to list..
     * @param value variable value
     */
    void addIntValue(IntType_t value);

    /**
     * @short Add value to list..
     * @param value variable value
     */
    void addRealValue(double value);

    /**
     * @short Add value to list..
     * @param value variable value
     */
    void addStringValue(const std::string &value) {
        addValue(value);
    }

    /**
     * @short Add value to list..
     * @param value variable value
     */
    void addFragmentValue(Fragment_t &&value) {
        addValue(std::move(value));
    }

    /**
     * @short Add value to list..
     * @param value variable value
     */
    void addFragmentListValue(FragmentList_t &&value) {
        addValue(std::move(value));
    }

    /**
     * @short Add some frag value to fragment.
     * @param value variable value
     */
    void addValue(Fragment_t &&value);

    /**
     * @short Add some frag value to fragment.
     * @param value variable value
     */
    void addValue(FragmentList_t &&value);

    /**
     * @short Add some frag value to fragment.
     * @param value variable value
     */
    void addValue(FragmentValue_t &&value);

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
    bool empty() const {return items.empty();}

    /**
     * @short Returns iterator to first fragment item.
     */
    const_iterator begin() const {return items.begin();}

    /**
     * @short Returns iterator one past the last fragment item.
     */
    const_iterator end() const {return items.end();}

    /**
     * @short Returns iterator to first fragment item.
     */
    iterator begin() {return items.begin();}

    /**
     * @short Returns iterator one past the last fragment item.
     */
    iterator end() {return items.end();}

    /**
     * @short Returns i-th fragment in the list.
     */
    const FragmentValue_t &operator[](size_type i) const {return items[i];}

    /**
     * @short Returns i-th fragment in the list.
     */
    FragmentValue_t &operator[](size_type i) {return items[i];}

protected:
    Items_t items; //!< the fragment list items
};

/** Writes string representation of fragment value list to stream.
 */
inline std::ostream &operator<<(std::ostream &os, const FragmentList_t &list) {
    list.dump(os);
    return os;
}

} // namespace Teng

#endif /* TENGFRAGMENTLIST_H */

