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
 * Teng syntax analyzer.
 *
 * AUTHORS
 * Filip Volejnik <filip.volejnik@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz
 *
 * HISTORY
 * 2012-04-16  (volejnik)
 *             First version.
 * 2018-07-07  (burlog)
 *             Cleaned.
 */

#ifndef TENGYYSTYPE_H
#define TENGYYSTYPE_H

#include <vector>
#include <string>

// TODO(burlog): remove it?
#include <iostream>

#include "tengposition.h"
#include "tengstringview.h"

namespace Teng {
namespace Parser {

/** Converts symbol id to its string representation.
 */
const char *symbol_name(int symbol_id);

/** Represents value of syntactic symbols. It is used for terminal and
 * nonterminal symbols in bison grammer.
 */
class Symbol_t {
public:
    Symbol_t(): pos(), id(), symbol_string() { std::cout << __PRETTY_FUNCTION__ << ":" << __FILE__ << ":" << __LINE__ << std::endl;}

    Symbol_t(Pos_t pos, int id, string_view_t symbol_string)
        : pos(pos), id(id), symbol_string(symbol_string)
    { std::cout << __PRETTY_FUNCTION__ << ":" << __FILE__ << ":" << __LINE__ << std::endl; }

    Symbol_t(const Symbol_t &other)
        : pos(other.pos), id(other.id), symbol_string(other.symbol_string)
    { std::cout << __PRETTY_FUNCTION__ << ":" << __FILE__ << ":" << __LINE__ << std::endl;}

    Symbol_t &operator=(const Symbol_t &other) {
        std::cout << __PRETTY_FUNCTION__ << ":" << __FILE__ << ":" << __LINE__ << std::endl;
        if (this != &other) {
            pos = other.pos;
            pos.colno = 333;
            id = other.id;
            symbol_string = other.symbol_string;
        }
        return *this;
    }

    Symbol_t(Symbol_t &&other)
        : pos(other.pos), id(other.id), symbol_string(other.symbol_string)
    { std::cout << __PRETTY_FUNCTION__ << ":" << __FILE__ << ":" << __LINE__ << std::endl;}

    Symbol_t &operator=(Symbol_t &&other) {
        std::cout << __PRETTY_FUNCTION__ << ":" << __FILE__ << ":" << __LINE__ << std::endl;
        if (this != &other) {
            pos = other.pos;
            id = other.id;
            symbol_string = other.symbol_string;
        }
        return *this;
    }

    ~Symbol_t() { std::cout << __PRETTY_FUNCTION__ << ":" << __FILE__ << ":" << __LINE__ << std::endl;}

    /** Returns string representation of the symbol id.
     */
    const char *name() const {return Parser::symbol_name(id);}

    /** Converts string view representing symbol to std::string.
     */
    std::string str() const {return symbol_string.str();}

    Pos_t pos;
    int id;
    string_view_t symbol_string;

    // enum class tag {integral, real, string, options, } symbol_tag;
    // union {
    // };

    // // define option list type
    // using OptionList_t = std::map<std::string, std::string>;
    // // teng-directive options -- used for building code
    // OptionList_t opt;
    //
    // // define list of addresses info
    // using AddressList_t = std::vector<uint64_t>;
    // // program address -- tmp just for building code
    // AddressList_t addr;
    //
    // // define identifier type
    // using Identifier_t = std::vector<std::string>;
    // // variable identifier
    // Identifier_t id;
    //
    // #<{(|* Sets identifier to {prefix0, ..., prefixn, name} and val to
    //  * prefix0.[...].prefix1.name string.
    //  |)}>#
    // void set_ident(Identifier_t prefix, std::string name = {}) {
    //     id = std::move(prefix);
    //     if (!name.empty())
    //         id.push_back(std::move(name));
    //     buildVal();
    // }
    //
    // #<{(|* Sets identifier to {lhs0, ..., lhsn, rhs0, ..., rhsn}
    //  * and val to lhs0.[...].lhsn.rhs0.[...].rhsn string.
    //  |)}>#
    // void set_ident(Identifier_t lhs, const Identifier_t &rhs) {
    //     id = std::move(lhs);
    //     for (auto item: rhs) id.push_back(item);
    //     buildVal();
    // }
    //
    // #<{(|* Sets val according to id value.
    //  |)}>#
    // void buildVal() {
    //     std::string tmp;
    //     for (auto &item: id) tmp += "." + item;
    //     val = std::move(tmp);
    // }
    //
    // // position in input stream (just for lexical elements)
    // // in other cases is pos-value irrelevant
    // Pos_t pos;
    //
    // // program size in the time when the element was read (and then shifted)
    // // this value is used for discarding parts of program in case of error
    // std::size_t prgsize;
};

} // namespace Parser
} // namespace Teng

#endif // TENGYYSTYPE_H

