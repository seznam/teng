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
 * $Id: tenglex1.h,v 1.3 2006-10-18 08:31:09 vasek Exp $
 *
 * DESCRIPTION
 * Teng helpers for FLEX.
 *
 * AUTHORS
 * Stepan Skrob <stepan@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-18  (stepan)
 *             Created.
 * 2018-07-07  (burlog)
 *             Cleaned.
 */

#ifndef TENGHELPERSEX1_H
#define TENGHELPERSEX1_H

#include <string>

#include "teng/stringview.h"

namespace Teng {
namespace Parser {

/** String values used for flex scanner requiring that buffers must ends
 * with flex end-of-buffer (two) zero bytes.
 */
struct flex_string_value_t: public string_value_t {
    /** C'tor: allocates new buffer finished by two zero bytes.
     */
    explicit flex_string_value_t(std::size_t length)
        : string_value_t(new char[length + 2], length)
    {disposable_ptr[length] = disposable_ptr[length + 1 ] = '\0';}

    /** Returns really allocated size including end-of-buffer bytes.
     */
    std::size_t flex_size() const {return size() + 2;}
};

/** Mutable string view that guaranties availability of the two extra bytes
 * past the string view last byte for flex end-of-buffer zero bytes.
 */
struct flex_string_view_t: public mutable_string_view_t {
    /** C'tor: conversion.
     */
    explicit flex_string_view_t(flex_string_value_t &value)
        : mutable_string_view_t(value.data(), value.size()),
          recover(true)
    {
        eob[0] = std::exchange(ptr[len + 0], '\0');
        eob[1] = std::exchange(ptr[len + 1], '\0');
    }

    /** C'tor: conversion.
     */
    flex_string_view_t(
        flex_string_view_t &value,
        std::size_t offset,
        std::size_t limit
    ): mutable_string_view_t(
        value.data() + offset,
        value.data() + offset + limit
    ), recover(true) {
        eob[0] = std::exchange(ptr[len + 0], '\0');
        eob[1] = std::exchange(ptr[len + 1], '\0');
    }

    /** C'tor: move.
     */
    flex_string_view_t(flex_string_view_t &&other) noexcept
        : mutable_string_view_t(std::move(other)),
          recover(other.recover), eob{other.eob[0], other.eob[1]}
    {other.recover = false;}

    /** Assigment: move.
     */
    flex_string_view_t &operator=(flex_string_view_t &&other) noexcept {
        if (this != &other) {
            this->~flex_string_view_t();
            new (this) flex_string_view_t(std::move(other));
        }
        return *this;
    }

    /** D'tor.
     */
    ~flex_string_view_t() noexcept {if (recover) reset();}

    /** Brings the string to the original state.
     */
    void reset() {
        recover = false;
        ptr[len + 0] = eob[0];
        ptr[len + 1] = eob[1];
    }

    /** Returns saved eob characters.
     */
    const char *saved_eob() const {return eob;}

    /** Returns really allocated size including end-of-buffer bytes.
     */
    std::size_t flex_size() const {return size() + 2;}

protected:
    // don't copy
    flex_string_view_t(const flex_string_view_t &) = delete;
    flex_string_view_t &operator=(const flex_string_view_t &) = delete;

    bool recover; //!< recover the last two bytes replaced with flex EOB
    char eob[2];  //!< two bytes that have been replaced by EOB
};

} // namespace Parser
} // namespace Teng

#endif /* TENGFLEXHELPERS_H */

