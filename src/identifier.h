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
 * $Id: tenginstruction.h,v 1.6 2010-06-11 07:46:26 burlog Exp $
 *
 * DESCRIPTION
 * Teng identifier for teng processor.
 *
 * AUTHORS
 * Stepan Skrob <stepan@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-20  (stepan)
 *             Created.
 * 2018-07-07  (burlog)
 *             Extracted from tenginstruction.cc.
 */

#ifndef TENGIDENTIFIER_H
#define TENGIDENTIFIER_H

#include <string>
#include <vector>

#include "lex2.h"
#include "teng/stringview.h"

namespace Teng {

/** Represents the Teng variable/function identifier.
 */
struct Identifier_t {
public:
    // types
    using Segments_t = std::vector<Parser::Token_t>;
    using const_iterator = Segments_t::const_iterator;
    using const_reverse_iterator = Segments_t::const_reverse_iterator;

    /** C'tor: default.
     */
    Identifier_t()
        : relative(true)
    {}

    /** C'tor: for setting relative/absolute.
     */
    explicit Identifier_t(bool relative)
        : relative(relative)
    {}

    /** C'tor: for relative from name.
     */
    Identifier_t(const Parser::Token_t &name)
        : relative(true), path{name}
    {}

    /** C'tor: from segments.
     */
    Identifier_t(Segments_t path, bool relative = false)
        : relative(relative), path(std::move(path))
    {}

    /** C'tor: from iter to segments.
     */
    template <typename iter_t>
    Identifier_t(iter_t ipos, iter_t epos, bool relative = false)
        : relative(relative), path(ipos, epos)
    {}

    /** Returns the number of identifier path segments.
     */
    std::size_t size() const {return path.size();}

    /** Returns true if identifier contains no segments.
     */
    bool empty() const {return path.empty();}

    /** Returns the last segment.
     */
    const Parser::Token_t &name() const {return path.back();}

    /** Returns iterator to the first identifier path segment.
     */
    const_iterator begin() const {return path.begin();}

    /** Returns iterator one past the last identifier path segment.
     */
    const_iterator end() const {return path.end();}

    /** Returns iterator to the last identifier path segment.
     */
    const_reverse_iterator rbegin() const {return path.rbegin();}

    /** Returns iterator one before the first identifier path segment.
     */
    const_reverse_iterator rend() const {return path.rend();}

    /** Returns true if identifier is relative.
     */
    bool is_relative() const {return relative;}

    /** Returns true if identifier is absolute.
     */
    bool is_absolute() const {return !relative;}

    /** Returns true if identifier is relative and contains only one segment -
     * the variable name.
     */
    bool is_local() const {return relative && (path.size() == 1);}

    /** Appends new segment at the end of identifier path.
     */
    void push_back(const Parser::Token_t &segment) {path.push_back(segment);}

    /** Removes last path segment.
     */
    void pop_back() {path.pop_back();}

    /** Returns i-th path segment.
     */
    const Parser::Token_t &operator[](std::size_t i) const {return path[i];}

    /** Comparison.
     */
    friend bool operator==(const Identifier_t &lhs, const Identifier_t &rhs);

    /** Returns all path segments joined by dot. If identifier is absolute then
     * it starts with dot.
     */
    std::string str() const {
        std::string result;
        if (is_absolute()) result.push_back('.');
        for (auto i = 0lu; i < size(); ++i)
            result.append((i? ".": "") + path[i].view());
        return result;
    }

protected:
    bool relative;   //!< true for relative identifiers
    Segments_t path; //!< the identifier segments
};

/** Comparison.
 */
inline bool operator==(const Identifier_t &lhs, const Identifier_t &rhs) {
    if (lhs.relative != rhs.relative)
        return false;
    if (lhs.size() != rhs.size())
        return false;
    for (std::size_t i = 0; i < lhs.path.size(); ++i)
        if (lhs.path[i].view() != rhs.path[i].view())
            return false;
    return true;
}

/** Comparison.
 */
inline bool operator!=(const Identifier_t &lhs, const Identifier_t &rhs) {
    return !(lhs == rhs);
}

/** Writes human readable string to stream.
 */
inline std::ostream &operator<<(std::ostream &os, const Identifier_t &ident) {
    os << ident.str();
    return os;
}

} // namespace Teng

#endif // TENGIDENTIFIER_H

