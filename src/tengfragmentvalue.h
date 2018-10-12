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

#ifndef TENGFRAGMENTVALUE_H
#define TENGFRAGMENTVALUE_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

#include <tengconfig.h>
#include <tengfragment.h>
#include <tengfragmentlist.h>

namespace Teng {

/** The reason of this struct is lack of explicit template parameters of c'tors
 * in C++.
 */
template <typename>
struct TypeTag_t {};

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

    // types
    enum class tag {frag, frag_ptr, frags, integral, real, string};

    /**
     * @short C'tor: move.
     */
    FragmentValue_t(FragmentValue_t &&other) noexcept;

    /**
     * @short Assigment: move.
     */
    FragmentValue_t &operator=(FragmentValue_t &&other) noexcept;

    /**
     * @short Create new scalar value with given value.
     * @param value value of variable
     */
    explicit FragmentValue_t(std::string value) noexcept
        : tag_value(tag::string), string_value(std::move(value))
    {}

    /**
     * @short Create new scalar value with given value.
     * @param value value of variable
     */
    explicit FragmentValue_t(IntType_t value) noexcept
        : tag_value(tag::integral), integral_value(value)
    {}

    /**
     * @short Create new scalar value with given value.
     * @param value value of variable
     */
    explicit FragmentValue_t(double value) noexcept
        : tag_value(tag::real), real_value(value)
    {}

    /** C'tor.
     */
    explicit FragmentValue_t(TypeTag_t<Fragment_t>) noexcept
        : tag_value(tag::frag), frag_value()
    {}

    /**
     * @short Create empty fragment list value.
     */
    explicit FragmentValue_t(TypeTag_t<FragmentList_t>) noexcept
        : tag_value(tag::frags), frags_value()
    {}

    /**
     * @short Destroy value.
     */
    ~FragmentValue_t() noexcept;

    /**
     * @short Sets value to string.
     */
    void setValue(const std::string &new_value);

    /**
     * @short Sets value to int.
     */
    template <
        typename type_t,
        std::enable_if_t<std::is_integral<type_t>::value, bool> = true
    > void setValue(type_t new_value) {setInt(new_value);}

    /**
     * @short Sets value to double.
     */
    template <
        typename type_t,
        std::enable_if_t<std::is_floating_point<type_t>::value, bool> = true
    > void setValue(type_t new_value) {setDouble(new_value);}

    /**
     * @short Sets value to int.
     */
    void setInt(IntType_t new_value);

    /**
     * @short Sets value to double.
     */
    void setDouble(double new_value);

    /**
     * @short Adds new empty fragment to the frament list.
     * @return new fragment
     */
    Fragment_t &addFragment();

    /**
     * @short Add new nested fragment list.
     * @param name fragment name
     * @return created fragment list
     *
     * Only if isNaturalDataStructuresEnabled() == true.
     */
    FragmentList_t &addFragmentList(const std::string &name);

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
     * @short Returns true if value is leaf => string/integral/real.
     */
    bool scalar() const {
        switch (tag_value) {
        case tag::frag: return false;
        case tag::frag_ptr: return false;
        case tag::frags: return false;
        case tag::integral: return true;
        case tag::real: return true;
        case tag::string: return true;
        }
    }

    /** Returns backward compatible string representation of value:
     *
     * - for scalar values it returns value converted to string,
     * - for non scalar values ti return empty string.
     */
    std::string toString() const;

    /**
     * @short Returns type of value.
     */
    tag type() const {return tag_value;}

    /**
     * @short Returns pointer to scallar value or nullptr.
     */
    const std::string *string() const {
        return tag_value == tag::string? &string_value: nullptr;
    }

    /**
     * @short Returns pointer to scallar value or nullptr.
     */
    const IntType_t *integral() const {
        return tag_value == tag::integral? &integral_value: nullptr;
    }

    /**
     * @short Returns pointer to scallar value or nullptr.
     */
    const double *real() const {
        return tag_value == tag::real? &real_value: nullptr;
    }

    /**
     * @short Returns pointer to list of values or nullptr.
     */
    const FragmentList_t *list() const {
        return tag_value == tag::frags? &frags_value: nullptr;
    }

    /**
     * @short Returns pointer to fragment or nullptr.
     */
    const Fragment_t *fragment() const {
        switch (tag_value) {
        case tag::frag: return &frag_value;
        case tag::frag_ptr: return ptr_value;
        default: return nullptr;
        }
    }

protected:
    // my close friends
    friend class Teng_t;
    friend class Value_t;
    friend struct OpenFragment_t;
    friend FragmentList_t;
    friend Fragment_t;

    /**
     * @short Create new scalar value with given value.
     * @param value value of variable
     */
    explicit FragmentValue_t(const Fragment_t *ptr_value) noexcept
        : tag_value(tag::frag_ptr), ptr_value(ptr_value)
    {}

    /**
     * @short Ensures that value is fragment list and if not then other value
     * is destroyed and new empty fragment list is assigned to value and
     * returned.
     */
    FragmentList_t &ensureFragmentList();

    /**
     * @short Ensures that value is fragment and if not then other value
     * is destroyed and new empty fragment is assigned to value and
     * returned.
     */
    Fragment_t &ensureFragment();

    tag tag_value; //!< the type of value
    union {
        std::string string_value;    //!< string (scalar) value
        IntType_t integral_value;    //!< integral number (scalar) value
        double real_value;           //!< real number (scalar) value
        FragmentList_t frags_value;  //!< list of nested fragment values
        Fragment_t frag_value;       //!< data fragment
        const Fragment_t *ptr_value; //!< for data root
    };
};

/** Writes string representation of fragment value to stream.
 */
inline std::ostream &operator<<(std::ostream &os, const FragmentValue_t &val) {
    val.dump(os);
    return os;
}

} // namespace Teng

#endif /* TENGFRAGMENTVALUE_H */

