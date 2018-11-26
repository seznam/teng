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
 * $Id: tengvalue.h,v 1.3 2007-05-21 15:43:28 vasek Exp $
 *
 * DESCRIPTION
 * Teng variables type.
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

#ifndef TENGVALUE_H
#define TENGVALUE_H

#include <string>
#include <stdexcept>

#include <teng/config.h>
#include <teng/stringify.h>
#include <teng/stringview.h>
#include <teng/counted_ptr.h>

namespace Teng {

// forwards
class Regex_t;
class Fragment_t;
class FragmentList_t;
class FragmentValue_t;

/** The type of undefined value.
 */
struct Undefined_t {};

/** Variant like class for Teng values that can hold numeric and string value.
 */
class Value_t {
public:
    // types
    using undefined_type = Undefined_t;
    using int_type = IntType_t;
    using real_type = double;
    using string_type = std::string;
    using string_ref_type = string_view_t;
    struct frag_ref_type {const Fragment_t *ptr;};
    struct list_ref_type {const FragmentList_t *ptr; std::size_t i;};
    using regex_type = counted_ptr<Regex_t>;

    /** Tags of all possible held value types.
     */
    enum class tag {
        undefined, string, string_ref, integral, real, frag_ref, list_ref,
        regex
    };

    /** Is used to mark type of the value that has been converted to printable
     * representation and passed to visitor.
     */
    template <tag> struct visited_type {};

    /** Returns tag value of visited type.
     */
    template <tag tag_v>
    static constexpr tag visited_value(visited_type<tag_v>) {return tag_v;}

    /** C'tor.
     */
    explicit Value_t(undefined_type = {}) noexcept
        : tag_value(tag::undefined), undefined_value()
    {}

    /** C'tor.
     */
    explicit Value_t(string_type value) noexcept
        : tag_value(tag::string), string_value(std::move(value))
    {}

    /** C'tor.
     */
    explicit Value_t(const char *value)
        : tag_value(tag::string), string_value(value)
    {}

    /** C'tor.
     */
    explicit Value_t(const string_ref_type &value) noexcept
        : tag_value(tag::string_ref), string_ref_value(value)
    {}

    /** C'tor.
     */
    explicit Value_t(regex_type value) noexcept
        : tag_value(tag::regex), undefined_value()
    {assign_regex(std::move(value));}

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

    /** C'tor: fragment value.
     */
    explicit Value_t(const Fragment_t *value) noexcept
        : tag_value(tag::frag_ref), frag_ref_value({value})
    {}

    /** C'tor: fragment value.
     */
    explicit Value_t(const FragmentList_t *value, std::size_t i = 0) noexcept
        : tag_value(tag::list_ref), list_ref_value({value, i})
    {}

    /** C'tor: fragment value.
     */
    explicit Value_t(const FragmentValue_t *value) noexcept;

    /** C'tor: copy.
     */
    Value_t(const Value_t &other)
        : tag_value(other.tag_value)
    {
        switch (other.tag_value) {
        case tag::undefined:
            break;
        case tag::integral:
            integral_value = other.integral_value;
            break;
        case tag::real:
            real_value = other.real_value;
            break;
        case tag::string:
            new (&string_value) string_type(other.string_value);
            break;
        case tag::string_ref:
            string_ref_value = other.string_ref_value;
            break;
        case tag::frag_ref:
            frag_ref_value = other.frag_ref_value;
            break;
        case tag::list_ref:
            list_ref_value = other.list_ref_value;
            break;
        case tag::regex:
            new (&regex_value) regex_type(other.regex_value);
            break;
        }
    }

    /** C'tor: move.
     */
    Value_t(Value_t &&other) noexcept
        : tag_value(other.tag_value)
    {
        switch (other.tag_value) {
        case tag::undefined:
            break;
        case tag::integral:
            integral_value = other.integral_value;
            break;
        case tag::real:
            real_value = other.real_value;
            break;
        case tag::string:
            new (&string_value) string_type(std::move(other.string_value));
            break;
        case tag::string_ref:
            string_ref_value = other.string_ref_value;
            break;
        case tag::frag_ref:
            frag_ref_value = other.frag_ref_value;
            break;
        case tag::list_ref:
            list_ref_value = other.list_ref_value;
            break;
        case tag::regex:
            new (&regex_value) regex_type(std::move(other.regex_value));
            break;
        }
    }

    /** Assigment operator: copy.
     */
    Value_t &operator=(const Value_t &other) {
        if (this != &other) {
            switch (other.tag_value) {
            case tag::undefined:
                operator=(undefined_type());
                break;
            case tag::integral:
                operator=(other.integral_value);
                break;
            case tag::real:
                operator=(other.real_value);
                break;
            case tag::string:
                operator=(other.string_value);
                break;
            case tag::string_ref:
                operator=(other.string_ref_value);
                break;
            case tag::frag_ref:
                dispose();
                frag_ref_value = other.frag_ref_value;
                tag_value = tag::frag_ref;
                break;
            case tag::list_ref:
                dispose();
                list_ref_value = other.list_ref_value;
                tag_value = tag::list_ref;
                break;
            case tag::regex:
                operator=(other.regex_value);
                break;
            }
        }
        return *this;
    }

    /** Assigment operator: move.
     */
    Value_t &operator=(Value_t &&other) noexcept {
        if (this != &other) {
            switch (other.tag_value) {
            case tag::undefined:
                operator=(undefined_type());
                break;
            case tag::integral:
                operator=(other.integral_value);
                break;
            case tag::real:
                operator=(other.real_value);
                break;
            case tag::string:
                operator=(std::move(other.string_value));
                break;
            case tag::string_ref:
                operator=(other.string_ref_value);
                break;
            case tag::frag_ref:
                dispose();
                frag_ref_value = other.frag_ref_value;
                tag_value = tag::frag_ref;
                break;
            case tag::list_ref:
                dispose();
                list_ref_value = other.list_ref_value;
                tag_value = tag::list_ref;
                break;
            case tag::regex:
                operator=(std::move(other.regex_value));
                break;
            }
        }
        return *this;
    }

    /** Assigment operator: undefined.
     */
    Value_t &operator=(undefined_type) noexcept {
        dispose();
        tag_value = tag::undefined;
        return *this;
    }

    /** Assigment operator: const char * copy.
     */
    Value_t &operator=(const char *value) {
        switch (tag_value) {
        case tag::string:
            string_value = value;
            break;
        case tag::regex:
            dispose_regex(regex_value);
        default:
            new (&string_value) string_type(value);
            tag_value = tag::string;
            break;
        }
        return *this;
    }

    /** Assigment operator: string copy.
     */
    Value_t &operator=(const string_type &value) {
        switch (tag_value) {
        case tag::string:
            string_value = value;
            break;
        case tag::regex:
            dispose_regex(regex_value);
        default:
            new (&string_value) string_type(value);
            tag_value = tag::string;
            break;
        }
        return *this;
    }

    /** Assigment operator: string move.
     */
    Value_t &operator=(string_type &&value) noexcept {
        switch (tag_value) {
        case tag::string:
            string_value = std::move(value);
            break;
        case tag::regex:
            dispose_regex(regex_value);
        default:
            new (&string_value) string_type(std::move(value));
            tag_value = tag::string;
            break;
        }
        return *this;
    }

    /** Assigment operator: string copy.
     */
    Value_t &operator=(const string_ref_type &value) noexcept {
        dispose();
        new (&string_ref_value) string_ref_type(value);
        tag_value = tag::string_ref;
        return *this;
    }

    /** Assigment operator: int.
     */
    template <
        typename type_t,
        std::enable_if_t<std::is_integral<type_t>::value, bool> = true
    > Value_t &operator=(type_t value) noexcept {
        dispose();
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
        dispose();
        real_value = value;
        tag_value = tag::real;
        return *this;
    }

    /** Assigment operator: regex copy.
     */
    Value_t &operator=(const regex_type &value) {
        switch (tag_value) {
        case tag::regex:
            assign_regex(value);
            break;
        case tag::string:
            string_value.~string_type();
        default:
            new (&regex_value) regex_type(value);
            tag_value = tag::regex;
            break;
        }
        return *this;
    }

    /** Assigment operator: regex move.
     */
    Value_t &operator=(regex_type &&value) noexcept {
        switch (tag_value) {
        case tag::regex:
            assign_regex(std::move(value));
            break;
        case tag::string:
            string_value.~string_type();
        default:
            new (&regex_value) regex_type(std::move(value));
            tag_value = tag::regex;
            break;
        }
        return *this;
    }

    /** D'tor.
     */
    ~Value_t() noexcept {dispose();}

    /** Retutns type of held value.
     */
    tag type() const {return tag_value;}

    /** Converts the type of held value to string.
     */
    std::string type_str() const {
        switch (tag_value) {
        case tag::undefined:
            return "undefined";
        case tag::integral:
            return "integral";
        case tag::real:
            return "real";
        case tag::string:
            return "string";
        case tag::string_ref:
            return "string_ref";
        case tag::frag_ref:
            return "frag_ref";
        case tag::list_ref:
            return "list_ref";
        case tag::regex:
            return "regex";
        }
        throw std::runtime_error(__PRETTY_FUNCTION__);
    }

    /** Returns true if value type is undefined.
     */
    bool is_undefined() const {return tag_value == tag::undefined;}

    /** Returns true if value type is integer.
     */
    bool is_integral() const {return tag_value == tag::integral;}

    /** Returns true if value type is real.
     */
    bool is_real() const {return tag_value == tag::real;}

    /** Returns true if value type is string.
     */
    bool is_string() const {return tag_value == tag::string;}

    /** Returns true if value type is string_ref.
     */
    bool is_string_ref() const {return tag_value == tag::string_ref;}

    /** Returns true if value type is frag ref.
     */
    bool is_frag_ref() const {return tag_value == tag::frag_ref;}

    /** Returns true if value type is frag list.
     */
    bool is_list_ref() const {return tag_value == tag::list_ref;}

    /** Returns true if value type is regex.
     */
    bool is_regex() const {return tag_value == tag::regex;}

    /** Returns integer value. Does not any checks.
     */
    int_type as_int() const {return integral_value;}

    /** Returns real value. Does not any checks.
     */
    real_type as_real() const {return real_value;}

    /** Returns string value. Does not any checks.
     */
    const std::string &as_string() const {return string_value;}

    /** Returns string value. Does not any checks.
     */
    const string_view_t &as_string_ref() const {return string_ref_value;}

    /** Returns frag value. Does not any checks.
     */
    const frag_ref_type &as_frag_ref() const {return frag_ref_value;}

    /** Returns list value. Does not any checks.
     */
    const list_ref_type &as_list_ref() const {return list_ref_value;}

    /** Returns list value. Does not any checks.
     */
    list_ref_type &as_list_ref() {return list_ref_value;}

    /** Returns regex value. Does not any checks.
     */
    const regex_type &as_regex() const {return regex_value;}

    /** Returns regex value. Does not any checks.
     */
    regex_type &as_regex() {return regex_value;}

    /** Returns true if value is integral or real number.
     */
    bool is_number() const {
        switch (tag_value) {
        case tag::real:
        case tag::integral:
            return true;
        default:
            return false;
        }
    }

    /** Returns true if value is string or string_ref.
     */
    bool is_string_like() const {
        switch (tag_value) {
        case tag::string:
        case tag::string_ref:
            return true;
        default:
            return false;
        }
    }

    /** Returns integral value or zero.
     */
    int_type integral() const {
        switch (tag_value) {
        case tag::integral:
            return integral_value;
        case tag::real:
            return static_cast<int_type>(real_value);
        default:
            return 0;
        }
    }

    /** Returns real value or zero.
     */
    real_type real() const {
        switch (tag_value) {
        case tag::integral:
            return static_cast<real_type>(integral_value);
        case tag::real:
            return real_value;
        default:
            return 0;
        }
    }

    /** Returns string value or empty string.
     */
    string_view_t string() const {
        switch (tag_value) {
        case tag::string:
            return string_value;
        case tag::string_ref:
            return string_ref_value;
        default:
            return {};
        }
    }

    /** Returns true if string value is not empty and numeric values are not
     * equal to zero.
     */
    explicit operator bool() const {
        switch (tag_value) {
        case tag::undefined:
            return false;
        case tag::integral:
            return integral_value;
        case tag::real:
            return real_value;
        case tag::string:
            return !string_value.empty();
        case tag::string_ref:
            return !string_ref_value.empty();
        case tag::frag_ref:
            return frag_ref_value.ptr;
        case tag::list_ref:
            return list_ref_value.ptr;
        case tag::regex:
            return true;
        }
        throw std::runtime_error(__PRETTY_FUNCTION__);
    }

    /** Converts value to json.
     */
    void json(std::ostream &out) const;

    /** Writes printable representation of value to writer.
     */
    template <typename Writer_t>
    auto print(Writer_t &&writer) const {
        return visit(*this, std::forward<Writer_t>(writer));
    }

    /** Appends the printable representation of the value, as it would be
     * printed, to given string.
     */
    void append_to(std::string &res) const {
        print([&] (const string_view_t &v) {res.append(v.data(), v.size());});
    }

    /** Returns the printable representation of the value, as it would be
     * printed.
     */
    std::string printable() const {
        return print([&] (const string_view_t &v) {return v.str();});
    }

    /** Appends string to string value. If needed, then the conversion to
     * printable representation is called.
     */
    Value_t &append_str(const string_view_t &suffix) {
        ensure_string().append(suffix.data(), suffix.size());
        return *this;
    }

    /** Appends printable value to string value. If needed, then the conversion
     * to printable representation is called.
     */
    Value_t &append_str(const Value_t &other) {
        other.print([&] (const string_view_t &v) {append_str(v);});
        return *this;
    }

    /** After calling this method, the value holds string or string_ref. If
     * needed, then the conversion to printable representation is called.
     */
    string_view_t ensure_string_like() {
        return visit(*this, [&] (const string_view_t &v, auto &&visit_tag) {
            switch (visited_value(visit_tag)) {
                case tag::string:
                    return string_view_t(string_value);
                case tag::string_ref:
                    return string_view_t(string_ref_value);
                case tag::regex:
                    dispose_regex(regex_value);
                    new (&string_value) string_type(v.str());
                    return string_view_t(string_value);
                default:
                    // calling the dispose() is not needed because only string
                    // and regex owns dynamic resources
                    new (&string_value) string_type(v.str());
                    return string_view_t(string_value);
            }
        });
    }

    /** After calling this method, the value holds string (even not
     * string_ref). If needed, then the conversion to printable representation
     * is called.
     */
    std::string &ensure_string() {
        visit(*this, [&] (const string_view_t &v, auto &&visit_tag) {
            if (visited_value(visit_tag) == tag::string)
                return;
            if (visited_value(visit_tag) == tag::regex)
                dispose_regex(regex_value);
            // here are all possible dynamic resources released
            tag_value = tag::string;
            new (&string_value) string_type(v.str());
        });
        return string_value;
    }

    /** Writes string representation to given stream.
     */
    friend std::ostream &operator<<(std::ostream &out, const Value_t &v);

    /** Unary minus operator.
     */
    friend inline Value_t operator-(const Value_t &value);

    /** Comparison operator.
    */
    friend inline bool operator==(const Value_t &lhs, const Value_t &rhs);

protected:
    /** Releases all allocated resources.
     */
    void dispose() {
        switch (tag_value) {
        case tag::string:
            string_value.~string_type();
            break;
        case tag::regex:
            dispose_regex(regex_value);
            break;
        default:
            /* no dynamic resources */
            break;
        }
    }

    /** Calls dtor of forward declared value.
     */
    static void dispose_regex(regex_type &regex_value);

    /** Assigns desired value to regex_value.
     */
    void assign_regex(const regex_type &value);

    /** Moves desired value to regex_value.
     */
    void assign_regex(regex_type &&value);

    /** Calls visitor with printable representation of value.
     */
    template <typename Self_t, typename Visitor_t>
    static auto visit(Self_t &&self, Visitor_t visitor)
    -> decltype(
        visitor(
            std::declval<string_view_t>(),
            std::declval<visited_type<tag::undefined>>()
        )
    ) {
        switch (self.tag_value) {
        case tag::undefined:
            static const string_view_t undefined = "undefined";
            static const visited_type<tag::undefined> tag_undefined;
            return visitor(undefined, tag_undefined);
        case tag::integral:
            static const visited_type<tag::integral> tag_integral;
            return stringify(self.integral_value, visitor, tag_integral);
        case tag::real:
            static const visited_type<tag::real> tag_real;
            return stringify(self.real_value, visitor, tag_real);
        case tag::string:
            static const visited_type<tag::string> tag_string;
            return visitor(self.string_value, tag_string);
        case tag::string_ref:
            static const visited_type<tag::string_ref> tag_string_ref;
            return visitor(self.string_ref_value, tag_string_ref);
        case tag::frag_ref:
            static const string_view_t null = "$null$";
            static const string_view_t frag = "$frag$";
            static const visited_type<tag::frag_ref> tag_frag_ref;
            return visitor(self.frag_ref_value.ptr? frag: null, tag_frag_ref);
        case tag::list_ref:
            static const string_view_t list = "$list$";
            static const visited_type<tag::list_ref> tag_list_ref;
            return visitor(self.list_ref_value.ptr? list: null, tag_list_ref);
        case tag::regex:
            static const string_view_t regex = "$regex$";
            static const visited_type<tag::undefined> tag_regex;
            return visitor(regex, tag_regex);
        }
        throw std::runtime_error(__PRETTY_FUNCTION__);
    }

    /** Calls visitor with printable representation of value.
     */
    template <typename Self_t, typename Visitor_t>
    static auto visit(Self_t &&self, Visitor_t visitor)
    -> decltype(visitor(std::declval<string_view_t>()))
    {return visit(self, [&] (auto &&v, auto) {return visitor(v);});}

    tag tag_value;                        //!< the type of value
    union {
        undefined_type undefined_value;   //!< undefined value
        int_type integral_value;          //!< integral number value
        real_type real_value;             //!< real number value
        string_type string_value;         //!< string value
        string_ref_type string_ref_value; //!< string view value
        frag_ref_type frag_ref_value;     //!< reference to frag
        list_ref_type list_ref_value;     //!< reference to fraglist
        regex_type regex_value;           //!< regular exprerssion pattern
    };
};

/** Unary minus operator.
 */
inline Value_t operator-(const Value_t &value) {
    switch (value.tag_value) {
    case Value_t::tag::integral:
        return Value_t(-value.as_int());
    case Value_t::tag::real:
        return Value_t(-value.as_real());
    default:
        return Value_t();
    }
}

/** Comparison operator.
 */
inline bool operator==(const Value_t &lhs, const Value_t &rhs) {
    switch (lhs.tag_value) {
    case Value_t::tag::undefined:
        return lhs.tag_value == rhs.tag_value;
    case Value_t::tag::integral:
        return lhs.tag_value == rhs.tag_value
            && lhs.integral_value == rhs.integral_value;
    case Value_t::tag::real:
        return lhs.tag_value == rhs.tag_value
            && lhs.real_value == rhs.real_value;
    case Value_t::tag::string:
        return lhs.string_value == rhs.string();
    case Value_t::tag::string_ref:
        return lhs.string_ref_value == rhs.string();
    case Value_t::tag::frag_ref:
        return lhs.tag_value == rhs.tag_value
            && lhs.frag_ref_value.ptr == rhs.frag_ref_value.ptr;
    case Value_t::tag::list_ref:
        return lhs.tag_value == rhs.tag_value
            && lhs.list_ref_value.ptr == rhs.list_ref_value.ptr
            && lhs.list_ref_value.i == rhs.list_ref_value.i;
    case Value_t::tag::regex:
        return lhs.tag_value == rhs.tag_value
            && lhs.regex_value == rhs.regex_value;
    }
    throw std::runtime_error(__PRETTY_FUNCTION__);
}

/** Comparison operator.
 */
inline bool operator!=(const Value_t &lhs, const Value_t &rhs) {
    return !(lhs == rhs);
}

} // namespace Teng

#endif /* TENGVALUE_H */

