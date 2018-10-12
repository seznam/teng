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
 * $Id$
 *
 * DESCRIPTION
 * String view and optionaly string holder.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz
 *
 * HISTORY
 * 2018-07-07  (burlog)
 *             First version.
 */

#ifndef TENGSTRINGVIEW_H
#define TENGSTRINGVIEW_H

#include <string>
#include <iosfwd>
#include <cstring>
#include <utility>

namespace Teng {

/** Represents part of string. It is used during parsing.
 */
template <typename ptr_type>
class string_view_facade_t {
public:
    /** C'tor: from cstring and size.
     */
    string_view_facade_t(ptr_type ptr, std::size_t len) noexcept
        : len(len), ptr(ptr)
    {}

    /** Returns pointer to the first character in view.
     */
    const char *data() const {return ptr;}

    /** Retutns the number character in view.
     */
    std::size_t size() const {return len;}

    /** Returns iterator to the first character in view.
     */
    const char *begin() const {return ptr;}

    /** Returns iterator one past the last character in the view.
     */
    const char *end() const {return ptr + len;}

    /** Conversion to std::string.
     */
    std::string str() const {return {ptr, len};}

    /** Returns character at the desired index.
     */
    char operator[](std::size_t i) const {return ptr[i];}

    /** Returns true if view isn't empty.
     */
    bool empty() const {return !len;}

    /** Returns the first character of view.
     */
    char front() const {return ptr[0];}

    /** Returns the last character of view.
     */
    char back() const {return ptr[len - 1];}

protected:
    std::size_t len; //!< the number of characters in view
    ptr_type ptr;    //!< pointer to first character in view
};

/** Returns true if two views represents same strings.
 */
template <typename lhs_char_t, typename rhs_char_t>
bool operator==(
    const string_view_facade_t<lhs_char_t> &lhs,
    const string_view_facade_t<rhs_char_t> &rhs
) {
    return (lhs.size() == rhs.size())
        && !std::char_traits<char>::compare(lhs.data(), rhs.data(), lhs.size());
}

/** Returns true if two views represents other strings.
 */
template <typename lhs_char_t, typename rhs_char_t>
bool operator!=(
    const string_view_facade_t<lhs_char_t> &lhs,
    const string_view_facade_t<rhs_char_t> &rhs
) {
    return !(lhs == rhs);
}

/** Returns true if two views represents same strings.
 */
template <typename lhs_char_t, typename rhs_char_t>
bool operator<(
    const string_view_facade_t<lhs_char_t> &lhs,
    const string_view_facade_t<rhs_char_t> &rhs
) {
    using traits = std::char_traits<char>;
    auto min_size = std::min(lhs.size(), rhs.size());
    if (auto res = traits::compare(lhs.data(), rhs.data(), min_size))
        return res < 0;
    return lhs.size() < rhs.size();
}

/** Returns true if two views represents same strings.
 */
template <typename lhs_char_t, typename rhs_char_t>
bool operator<=(
    const string_view_facade_t<lhs_char_t> &lhs,
    const string_view_facade_t<rhs_char_t> &rhs
) {
    return !(rhs < lhs);
}

/** Returns true if two views represents same strings.
 */
template <typename lhs_char_t, typename rhs_char_t>
bool operator>(
    const string_view_facade_t<lhs_char_t> &lhs,
    const string_view_facade_t<rhs_char_t> &rhs
) {
    return rhs < lhs;
}

/** Returns true if two views represents same strings.
 */
template <typename lhs_char_t, typename rhs_char_t>
bool operator>=(
    const string_view_facade_t<lhs_char_t> &lhs,
    const string_view_facade_t<rhs_char_t> &rhs
) {
    return !(lhs < rhs);
}

/** Concats the string view and std string.
 */
template <typename lhs_char_t>
std::string operator+(
    const string_view_facade_t<lhs_char_t> &lhs,
    const std::string &rhs
) {
    std::string result;
    result.reserve(lhs.size() + rhs.size());
    result.append(lhs.data(), lhs.size()).append(rhs);
    return result;
}

/** Concats the string view and std string.
 */
template <typename rhs_char_t>
std::string operator+(
    const std::string &lhs,
    const string_view_facade_t<rhs_char_t> &rhs
) {
    std::string result;
    result.reserve(lhs.size() + rhs.size());
    result.append(lhs).append(rhs.data(), rhs.size());
    return result;
}

/** Concats the string view and std string.
 */
template <typename lhs_char_t>
std::string operator+(
    const string_view_facade_t<lhs_char_t> &lhs,
    const string_view_facade_t<lhs_char_t> &rhs
) {
    std::string result;
    result.reserve(lhs.size() + rhs.size());
    result.append(lhs.data(), lhs.size()).append(rhs.data(), rhs.size());
    return result;
}

/** Concats the string view and std string.
 */
template <typename rhs_char_t>
std::string operator+=(
    std::string &lhs,
    const string_view_facade_t<rhs_char_t> &rhs
) {
    return lhs.append(rhs.data(), rhs.size());
}

/** Represent part of mutable string.
 */
class mutable_string_view_t: public string_view_facade_t<char *> {
public:
    /** C'tor: default.
     */
    mutable_string_view_t() noexcept
        : string_view_facade_t<char *>(nullptr, 0)
    {}

    /** C'tor: from cstring and size.
     */
    mutable_string_view_t(char *ptr, std::size_t len) noexcept
        : string_view_facade_t<char *>(ptr, len)
    {}

    /** C'tor: from two pointer.
     */
    mutable_string_view_t(char *start, char *end) noexcept
        : string_view_facade_t<char *>(start, end - start)
    {}

    /** C'tor: from cstring.
     */
    mutable_string_view_t(char *ptr) noexcept
        : string_view_facade_t<char *>(ptr, strlen(ptr))
    {}

    // make const variants visible
    using string_view_facade_t<char *>::data;
    using string_view_facade_t<char *>::begin;
    using string_view_facade_t<char *>::end;
    using string_view_facade_t<char *>::operator[];

    /** Returns pointer to the first character in view.
     */
    char *data() {return ptr;}

    /** Returns iterator to the first character in view.
     */
    char *begin() {return ptr;}

    /** Returns iterator one past the last character in the view.
     */
    char *end() {return ptr + len;}

    /** Returns character at the desired index.
     */
    char &operator[](std::size_t i) {return ptr[i];}
};

/** Represent part of imutable string.
 */
class string_view_t: public string_view_facade_t<const char *> {
public:
    /** C'tor: default.
     */
    string_view_t() noexcept
        : string_view_facade_t<const char *>(nullptr, 0)
    {}

    /** C'tor: from cstring and size.
     */
    string_view_t(const char *ptr, std::size_t len) noexcept
        : string_view_facade_t<const char *>(ptr, len)
    {}

    /** C'tor: from string.
     */
    string_view_t(const std::string &str) noexcept
        : string_view_facade_t<const char *>(str.c_str(), str.size())
    {}
    /** C'tor: from two pointer.
     */
    string_view_t(const char *start, const char *end) noexcept
        : string_view_facade_t<const char *>(start, end - start)
    {}

    /** C'tor: from cstring.
     */
    string_view_t(const char *ptr) noexcept
        : string_view_facade_t<const char *>(ptr, strlen(ptr))
    {}

    /** C'tor: from cstring.
     */
    string_view_t(const mutable_string_view_t &view) noexcept
        : string_view_facade_t<const char *>(view.data(), view.size())
    {}
};

/** Returns true if two views represents same strings.
 */
inline bool operator==(const string_view_t &lhs, const string_view_t &rhs) {
    return (lhs.size() == rhs.size())
        && !std::char_traits<char>::compare(lhs.data(), rhs.data(), lhs.size());
}

/** Returns true if two views represents other strings.
 */
inline bool operator!=(const string_view_t &lhs, const string_view_t &rhs) {
    return !(lhs == rhs);
}

/** Returns true if two views represents same strings.
 */
inline bool operator<(const string_view_t &lhs, const string_view_t &rhs) {
    using traits = std::char_traits<char>;
    auto min_size = std::min(lhs.size(), rhs.size());
    if (auto res = traits::compare(lhs.data(), rhs.data(), min_size))
        return res < 0;
    return lhs.size() < rhs.size();
}

/** Represent string view that optionaly can hold the data.
 */
class string_value_t: public string_view_t {
public:
    /** C'tor: default.
     */
    string_value_t(): string_view_t(), disposable_ptr(nullptr) {}

    /** C'tor: for eternal strings.
     */
    explicit string_value_t(const string_view_t &value)
        : string_view_t(value), disposable_ptr(nullptr)
    {}

    /** C'tor: allocates new buffer.
     */
    explicit string_value_t(std::size_t length)
        : string_value_t(new char[length], length)
    {}

    /** C'tor: move.
     */
    string_value_t(string_value_t &&other) noexcept
        : string_view_t(std::move(other)),
          disposable_ptr(std::exchange(other.disposable_ptr, nullptr))
    {}

    /** Assigment: move.
     */
    string_value_t &operator=(string_value_t &&other) noexcept {
        if (this != &other) {
            this->~string_value_t();
            new (this) string_value_t(std::move(other));
        }
        return *this;
    }

    /** D'tor.
     */
    ~string_value_t() noexcept {if (disposable_ptr) delete [] disposable_ptr;}

    // make const variants visible
    using string_view_t::data;
    using string_view_t::begin;
    using string_view_t::end;
    using string_view_t::operator[];

    /** Returns pointer to the first mutable character in view.
     */
    char *data() {return disposable_ptr;}

    /** Returns iterator to the first mutable character in view.
     */
    char *begin() {return disposable_ptr;}

    /** Returns iterator one past the last mutable character in the view.
     */
    char *end() {return disposable_ptr + len;}

    /** Returns mutable character at the desired index.
     */
    char operator[](std::size_t i) {return disposable_ptr[i];}

protected:
    // don't copy
    string_value_t(const string_value_t &) = delete;
    string_value_t &operator=(const string_value_t &) = delete;

    /** C'tor: for mortal strings.
     */
    string_value_t(char *ptr, std::size_t len)
        : string_view_t(ptr, len), disposable_ptr(ptr)
    {}

    char *disposable_ptr; //!< pointer to disposable data
};

/** Writes string to stream.
 */
std::ostream &operator<<(std::ostream &os, const string_view_t &view);

/** Writes string to stream.
 */
std::ostream &operator<<(std::ostream &os, const mutable_string_view_t &view);

} // namespace Teng

#endif // TENGSTRINGVIEW_H

