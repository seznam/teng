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
 * $Id: tengparservalue.h,v 1.3 2007-05-21 15:43:28 vasek Exp $
 *
 * DESCRIPTION
 * Teng data types.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-17  (vasek)
 *             Created.
 * 2018-06-07  (burlog)
 *             Make it more type safe.
 */

#ifndef TENGPARSERVALUE_H
#define TENGPARSERVALUE_H

#include <string>

#include <tengconfig.h>

// TODO(burlog): remove it
#include <iostream>

namespace Teng {

// forwards
class FragmentValue_t;

namespace Parser {

// TODO(burlog): move to Teng namespace

/** The undefined value.
 */
struct Undefined_t {};

/** Variant like class for Teng values that can hold numeric and string value.
 */
class Value_t {
public:
    // types
    using string = std::string;
    using int_type = IntType_t;
    using real_type = double;
    struct ptr_type {const void *ptr; int type;};
    enum class tag {undefined, string, integral, real, pointer};

    /** C'tor.
     */
    explicit Value_t(Undefined_t = {}) noexcept
        : tag_value(tag::undefined), undefined_value()
    {}

    /** C'tor.
     */
    explicit Value_t(const std::string &value)
        : tag_value(tag::string), string_value(value)
    {}

    /** C'tor.
     */
    template <
        typename type_t,
        std::enable_if_t<std::is_integral<type_t>::value, bool> = true
    > explicit Value_t(type_t value) noexcept
        : tag_value(tag::integral), integral_value(value)
    {}

    /** C'tor.
     */
    template <
        typename type_t,
        std::enable_if_t<std::is_floating_point<type_t>::value, bool> = true
    > explicit Value_t(type_t value) noexcept
        : tag_value(tag::real), real_value(value)
    {}

    /** C'tor.
     */
    explicit Value_t(const ptr_type &value) noexcept
        : tag_value(tag::pointer), ptr_value(value)
    {}

    /** C'tor.
     */
    template <typename type_t>
    Value_t(const void *ptr, type_t type) noexcept
        : tag_value(tag::pointer), ptr_value({ptr, static_cast<int>(type)})
    {}

    /** C'tor: copy.
     */
    Value_t(const Value_t &value)
        : tag_value(value.tag_value)
    {
        switch (value.tag_value) {
        case tag::string:
            new (&string_value) std::string(value.string_value);
            break;
        case tag::integral:
            integral_value = value.integral_value;
            break;
        case tag::real:
            real_value = value.real_value;
            break;
        case tag::undefined:
            break;
        case tag::pointer:
            ptr_value = value.ptr_value;
            break;
        }
    }

    /** C'tor: move.
     */
    Value_t(Value_t &&value) noexcept
        : tag_value(value.tag_value)
    {
        switch (value.tag_value) {
        case tag::string:
            new (&string_value) std::string(std::move(value.string_value));
            break;
        case tag::integral:
            integral_value = value.integral_value;
            break;
        case tag::real:
            real_value = value.real_value;
            break;
        case tag::undefined:
            break;
        case tag::pointer:
            ptr_value = value.ptr_value;
            break;
        }
    }

    /** Assigment operator: copy.
     */
    Value_t &operator=(const Value_t &value) {
        if (this != &value) {
            switch (value.tag_value) {
            case tag::string:
                *this = value.string_value;
                break;
            case tag::integral:
                *this = value.integral_value;
                break;
            case tag::real:
                *this = value.real_value;
                break;
            case tag::undefined:
                *this = Undefined_t{};
                break;
            case tag::pointer:
                *this = value.ptr_value;
                break;
            }
        }
        return *this;
    }

    /** Assigment operator: move.
     */
    Value_t &operator=(Value_t &&value) noexcept {
        if (this != &value) {
            switch (value.tag_value) {
            case tag::string:
                *this = std::move(value.string_value);
                break;
            case tag::integral:
                *this = value.integral_value;
                break;
            case tag::real:
                *this = value.real_value;
                break;
            case tag::undefined:
                *this = Undefined_t{};
                break;
            case tag::pointer:
                *this = value.ptr_value;
                break;
            }
        }
        return *this;
    }

    /** Assigment operator: undefined.
     */
    Value_t &operator=(Undefined_t) {
        using std::string;
        if (is_string()) string_value.~string();
        tag_value = tag::undefined;
        return *this;
    }

    /** Assigment operator: string copy.
     */
    Value_t &operator=(const std::string &value) {
        if (is_string()) string_value = value;
        else new (&string_value) std::string(value);
        tag_value = tag::string;
        return *this;
    }

    /** Assigment operator: string move.
     */
    Value_t &operator=(std::string &&value) noexcept {
        if (is_string()) string_value = std::move(value);
        else new (&string_value) std::string(std::move(value));
        tag_value = tag::string;
        return *this;
    }

    /** Assigment operator: int.
     */
    template <
        typename type_t,
        std::enable_if_t<std::is_integral<type_t>::value, bool> = true
    > Value_t &operator=(type_t value) noexcept {
        using std::string;
        if (is_string()) string_value.~string();
        integral_value = value;
        tag_value = tag::integral;
        return *this;
    }

    /** Assigment operator: real.
     */
    template <
        typename type_t,
        std::enable_if_t<std::is_floating_point<type_t>::value, bool> = true
    > Value_t &operator=(type_t value) noexcept {
        using std::string;
        if (is_string()) string_value.~string();
        real_value = value;
        tag_value = tag::real;
        return *this;
    }

    /** Assigment operator: string move.
     */
    Value_t &operator=(const ptr_type &value) noexcept {
        using std::string;
        if (is_string()) string_value.~string();
        ptr_value = value;
        tag_value = tag::pointer;
        return *this;
    }

    /** Assigment operator: fragment value.
     */
    Value_t &operator=(const FragmentValue_t &value) noexcept;

    /** D'tor.
     */
    ~Value_t() {if (is_string()) string_value.~basic_string();}

    /** Retutns type of held value.
     */
    tag type() const {return tag_value;}

    /** Converts the type of held value to string.
     */
    std::string type_str() const {
        switch (tag_value) {
        case tag::string:
            return "string";
        case tag::integral:
            return "integral";
        case tag::real:
            return "real";
        case tag::undefined:
            return "undefined";
        case tag::pointer:
            return "pointer";
        }
    }

    /** Returns true if value type is string.
     */
    bool is_undefined() const {return tag_value == tag::undefined;}

    /** Returns true if value type is string.
     */
    bool is_string() const {return tag_value == tag::string;}

    /** Returns true if value type is integer.
     */
    bool is_integral() const {return tag_value == tag::integral;}

    /** Returns true if value type is real.
     */
    bool is_real() const {return tag_value == tag::real;}

    /** Returns true if value type is real.
     */
    bool is_pointer() const {return tag_value == tag::pointer;}

    /** Returns true if value is integral or real number.
     */
    bool is_number() const {return is_real() || is_integral();}

    /** Returns string value. Does not any checks.
     */
    const std::string &as_str() const {return string_value;}

    /** Returns string value. Does not any checks.
     */
    std::string &as_str() {return string_value;}

    /** Returns integer value. Does not any checks.
     */
    int_type as_int() const {return integral_value;}

    /** Returns real value. Does not any checks.
     */
    real_type as_real() const {return real_value;}

    /** Returns pointer value. Does not any checks.
     */
    const ptr_type &as_ptr() const {return ptr_value;}

    /** Returns pointer. Does not any checks.
     */
    template <typename type_t>
    auto *pointer() const {return static_cast<const type_t *>(ptr_value.ptr);}

    /** Returns pointer. Does not any checks.
     */
    template <typename type_t>
    type_t pointer_type() const {return static_cast<type_t>(ptr_value.type);}

    /** Returns string representation of the given pointer.
     */
    std::string to_string(const ptr_type &value) const;

    /** Returns string representation of the value.
     */
    std::string str() const {
        switch (tag_value) {
        case tag::string:
            return string_value;
        case tag::integral:
            return std::to_string(integral_value);
        case tag::real:
            return std::to_string(real_value);
        case tag::undefined:
            return "undefined";
        case tag::pointer:
            return to_string(ptr_value);
        }
    }

    /** Returns string representation of the value.
     */
    const std::string &str(std::string &tmp) const {
        static const std::string undefined_str = "undefined";
        switch (tag_value) {
        case tag::string:
            return string_value;
        case tag::integral:
            return tmp = std::to_string(integral_value);
        case tag::real:
            return tmp = std::to_string(real_value);
        case tag::undefined:
            return undefined_str;
        case tag::pointer:
            return tmp = to_string(ptr_value);
        }
    }

    /** Returns integral representation of the value.
     */
    int_type integral() const {
        switch (tag_value) {
        case tag::string:
            return 0;
        case tag::integral:
            return integral_value;
        case tag::real:
            return real_value;
        case tag::undefined:
            return 0;
        case tag::pointer:
            return 0;
        }
    }

    /** Returns real representation of the value.
     */
    real_type real() const {
        switch (tag_value) {
        case tag::string:
            return 0;
        case tag::integral:
            return integral_value;
        case tag::real:
            return real_value;
        case tag::undefined:
            return 0;
        case tag::pointer:
            return 0;
        }
    }

    /** Returns true if string value is not empty and numeric values are not
     * equal to zero.
     */
    explicit operator bool() const {
        switch (tag_value) {
        case tag::string:
            return !string_value.empty();
        case tag::integral:
            return integral_value;
        case tag::real:
            return real_value;
        case tag::undefined:
            return false;
        case tag::pointer:
            return ptr_value.ptr;
        }
    }

    /** Writes string representation to given stream.
     */
    friend std::ostream &operator<<(std::ostream &o, const Value_t &v);

    /** Unary minus operator.
     */
    friend inline Value_t operator-(const Value_t &value);

protected:
    tag tag_value;                   //!< the type of value
    union {
        Undefined_t undefined_value; //!< undefined
        std::string string_value;    //!< value for strings
        int_type integral_value;     //!< value for integral numbers
        real_type real_value;        //!< value for real numbers
        ptr_type ptr_value;          //!< value for pointers
    };
};

/** Unary minus operator.
 */
inline Value_t operator-(const Value_t &value) {
    switch (value.tag_value) {
    case Value_t::tag::string:
        return Value_t(Undefined_t());
    case Value_t::tag::integral:
        return Value_t(-value.as_int());
    case Value_t::tag::real:
        return Value_t(-value.as_real());
    case Value_t::tag::undefined:
        return value;
    case Value_t::tag::pointer:
        return Value_t(Undefined_t());
    }
}

} // namespace Parser
} // namespace Teng

#endif // TENGPARSERVALUE_H

