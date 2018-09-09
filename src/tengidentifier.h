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

// TODO(burlog): remove
#include <iostream>

#include "tengstringview.h"

namespace Teng {

/** 
 */
struct Identifier_t {
public:
    // TODO(burlog): rename to Identifier_t!
    using const_iterator = std::vector<string_view_t>::const_iterator;
    using const_reverse_iterator = std::vector<string_view_t>::const_reverse_iterator;

    Identifier_t(): relative(true) {}
    Identifier_t(bool relative): relative(relative) {}
    Identifier_t(const string_view_t &name): relative(true), path{name} {}
    Identifier_t(std::vector<string_view_t> path, bool relative = false)
        : relative(relative), path(std::move(path))
    {}
    template <typename iter_t>
    Identifier_t(iter_t ipos, iter_t epos, bool relative = false)
        : relative(relative), path(ipos, epos)
    {}

    explicit operator bool() const {return !path.empty();}

    std::size_t size() const {return path.size();}

    bool empty() const {return path.empty();}

    const string_view_t &name() const {return path.back();}

    const_iterator begin() const {return path.begin();}
    const_iterator end() const {return path.end();}
    const_reverse_iterator rbegin() const {return path.rbegin();}
    const_reverse_iterator rend() const {return path.rend();}

    bool is_relative() const {return relative;}
    bool is_absolute() const {return !relative;}
    bool is_local() const {return relative && (path.size() == 1);}

    void push_back(const string_view_t &segment) {path.push_back(segment);}
    void pop_back() {path.pop_back();}

    const string_view_t &operator[](std::size_t i) const {return path[i];}

    friend bool operator==(const Identifier_t &lhs, const Identifier_t &rhs);

    std::string str() const {
        std::string result;
        if (is_absolute()) result.push_back('.');
        for (int i = 0; i < size(); ++i)
            result.append((i? ".": "") + path[i]);
        return result;
    }

protected:
    bool relative;
    std::vector<string_view_t> path;
};

/** 
 */
inline bool operator==(const Identifier_t &lhs, const Identifier_t &rhs) {
    return lhs.relative == rhs.relative
        && lhs.path == rhs.path;
}

/** 
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

