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
 * $Id: tengparsercontext.h,v 1.5 2006-10-18 08:31:09 vasek Exp $
 *
 * DESCRIPTION
 * Teng parser context.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-17  (vasek)
 *             Created.
 * 2018-07-07  (burlog)
 *             Extracted from tengparsercontext.cc.
 */

#ifndef TENGPARSERFRAG_H
#define TENGPARSERFRAG_H

#include <string>
#include <vector>

#include <tengidentifier.h>

namespace Teng {
namespace Parser {

/** This class represents stack of open fragments by frag Teng directive during
 * parsing of template. This is 2D structure because you can open root fragment
 * again and again. See this example:
 *
 * <?teng frag frag1?>
 * <?teng frag frag2?>
 * <?teng frag .frag1?>
 * ${var}
 * <?teng endfrag?>
 * <?teng endfrag?>
 * <?teng endfrag?>
 *
 * where within frag2 is root child frag1 again opened. As long as it is not
 * closed, it is necessary to maintain new path of open fragments.
 */
class FragmentContexts_t {
public:
    struct Entry_t {
        IdentifierPath_t ident;
        std::vector<int32_t> addrs;
    };

    Entry_t &top() {
        return ctxs.back();
    }

    std::vector<Entry_t> ctxs;


    // #<{(|* Var/frag identifier. |)}>#
    // using IdentifierName_t = std::vector<std::string>;
    //
    // #<{(|* 
    //  |)}>#
    // Identifier_t open(const Position_t &pos, const IdentifierName_t &name);
    //
    // #<{(|* 
    //  |)}>#
    // void close(unsigned int fragmentProgramStart);
    //
    // #<{(|* 
    //  |)}>#
    // bool findFragmentForVar(const Position_t &pos, const IdentifierName_t &name, Identifier_t &id) const;
    //
    // #<{(|* 
    //  |)}>#
    // FR findFragment(const Position_t *pos, const IdentifierName_t &name, Identifier_t &id, bool parentIsOK = false) const;
    //
    // #<{(|* 
    //  |)}>#
    // ER exists(const Position_t &pos, const IdentifierName_t &name, Identifier_t &id, bool mustBeOpen = false) const;
    //
    // #<{(|* 
    //  |)}>#
    // int getFragmentAddress(const Position_t &pos, const IdentifierName_t &name, Identifier_t &id) const;
    //
    // #<{(|* 
    //  |)}>#
    // struct FragmentContext_t {
    //     void reserve(unsigned int n) {
    //         name.reserve(n);
    //         addresses.reserve(n);
    //     }
    //
    //     std::string fullname() const;
    //
    //     void push_back(const std::string &n, int a) {
    //         name.push_back(n);
    //         addresses.push_back(a);
    //     }
    //
    //     void pop_back() {
    //         name.pop_back();
    //         addresses.pop_back();
    //     }
    //
    //     bool empty() const {
    //         return name.empty();
    //     }
    //
    //     unsigned int size() const {
    //         return name.size();
    //     }
    //
    //     operator const IdentifierName_t&() const {
    //         return name;
    //     }
    //
    //     int getAddress(const IdentifierName_t &id) const;
    //
    //     IdentifierName_t name;
    //     std::vector<int> addresses;
    // };
    //
    // #<{(|* Actual fragment context when parsing a template. |)}>#
    // std::vector<FragmentContext_t> fragContext;
};

} // namespace Parser
} // namespace Teng

#endif // TENGPARSERFRAG_H

