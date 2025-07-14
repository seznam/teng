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

#ifndef TENGFRAGMENT_H
#define TENGFRAGMENT_H

#include <map>
#include <string>
#include <cstdint>
#include <type_traits>

#include <teng/types.h>

namespace Teng {

// forwards
class Fragment_t;
class FragmentValue_t;
class FragmentList_t;

/** Transparent string comparator.
 */
struct StrCmp_t {
    template <typename Lhs_t, typename Rhs_t>
    bool operator()(Lhs_t &&lhs, Rhs_t &&rhs) const {return lhs < rhs;}
    struct is_transparent {};
};

/**
 * @short Single fragment. Maps names to variables and nested fragments.
 */
class Fragment_t {
public:
    // types
    using Item_t = FragmentValue_t;
    using Items_t = std::map<std::string, Item_t, StrCmp_t>;
    using const_iterator = Items_t::const_iterator;
    using iterator = Items_t::iterator;

    // don't copy
    Fragment_t(const Fragment_t &) = delete;
    Fragment_t &operator=(const Fragment_t &) = delete;

    /**
     * @short C'tor.
     */
    Fragment_t() noexcept = default;

    /**
     * @short C'tor: move.
     */
    Fragment_t(Fragment_t &&other) noexcept = default;

    /**
     * @short Assigment: move.
     */
    Fragment_t &operator=(Fragment_t &&other) noexcept = default;

    /** D'tor.
     */
    ~Fragment_t() noexcept = default;

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
    template <
        typename type_t,
        std::enable_if_t<std::is_integral<type_t>::value, bool> = true
    > void addVariable(const std::string &name, type_t value) {
        addIntVariable(name, value);
    }

    /**
     * @short Add variable to fragment.
     * @param name variable name
     * @param value variable value
     */
    template <
        typename type_t,
        std::enable_if_t<std::is_floating_point<type_t>::value, bool> = true
    > void addVariable(const std::string &name, type_t value) {
        addRealVariable(name, value);
    }

    /**
     * @short Add variable to fragment.
     * @param name variable name
     * @param value variable value
     */
    void addIntVariable(const std::string &name, IntType_t value);

    /**
     * @short Add variable to fragment.
     * @param name variable name
     * @param value variable value
     */
    void addRealVariable(const std::string &name, double value);

    /**
     * @short Add variable to fragment.
     * @param name variable name
     * @param value variable value
     */
    void addStringVariable(const std::string &name, const std::string &value) {
        addVariable(name, value);
    }

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
     * @short Add some frag value to fragment.
     * @param name variable name
     * @param value variable value
     */
    void addValue(const std::string &name, Fragment_t &&value);

    /**
     * @short Add some frag value to fragment.
     * @param name variable name
     * @param value variable value
     */
    void addValue(const std::string &name, FragmentList_t &&value);

    /**
     * @short Add some frag value to fragment.
     * @param name variable name
     * @param value variable value
     */
    void addValue(const std::string &name, FragmentValue_t &&value);

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
    template <typename Type_t>
    const_iterator find(Type_t &&name) const {return items.find(name);}

    /**
     * @short Returns iterator to fragment item of desired name.
     */
    iterator find(const std::string &name) {return items.find(name);}

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
     * @short Returns true if fragment is empty.
     */
    bool empty() const {return items.empty();}

    /**
     * @short Returns the number of elements in fragment.
     */
    std::size_t size() const {return items.size();}

protected:
    Items_t items; //!< fragments data
};

/** Writes string representation of fragment to stream.
 */
inline std::ostream &operator<<(std::ostream &os, const Fragment_t &frag) {
    frag.dump(os);
    return os;
}

} // namespace Teng

#endif /* TENGFRAGMENT_H */

