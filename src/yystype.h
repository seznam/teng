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
 * Teng syntax symbols.
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
#include <algorithm>

#include "lex2.h"
#include "position.h"
#include "identifier.h"
#include "teng/stringview.h"
#include "teng/value.h"

namespace Teng {
namespace Parser {

// forwards
struct Context_t;

/** The symbol wrapper that prevents the C++ bison parser to construct a large
 * amount of useless symbols that will immediately replaced with another values.
 */
template <typename ValueType_t>
class OptionalSymbol_t {
public:
    // we are enforcing nothrow move semantic
    static_assert(
        std::is_nothrow_move_constructible<ValueType_t>::value,
        "The value type has to be nothrow move constructible!"
    );
    static_assert(
        std::is_nothrow_destructible<ValueType_t>::value,
        "The value type has to be nothrow destructible!"
    );

    /** C'tor: default.
     */
    OptionalSymbol_t() noexcept
        : initialized(false)
    {}

    /** C'tor: copy.
     */
    OptionalSymbol_t(const OptionalSymbol_t &other)
        : initialized(other.initialized)
    {if (other.initialized) new (&value) ValueType_t(*other);}

    /** Assigment: copy.
     */
    OptionalSymbol_t &operator=(const OptionalSymbol_t &other) {
        if (this != &other) {
            if (other.initialized) {
                if (initialized) {
                    ValueType_t tmp(*other);
                    value.~ValueType_t();
                    new (&value) ValueType_t(std::move(tmp));
                } else new (&value) ValueType_t(*other);
            } else if (initialized) value.~ValueType_t();
            initialized = other.initialized;
        }
        return *this;
    }

    /** C'tor: move
     */
    OptionalSymbol_t(OptionalSymbol_t &&other) noexcept
        : initialized(other.initialized)
    {if (other.initialized) new (&value) ValueType_t(std::move(*other));}

    /** Assigment: move.
     */
    OptionalSymbol_t &operator=(OptionalSymbol_t &&other) noexcept {
        if (this != &other) {
            if (initialized)
                value.~ValueType_t();
            if (other.initialized)
                new (&value) ValueType_t(std::move(*other));
            initialized = other.initialized;
        }
        return *this;
    }

    /** Creates symbol value inplace.
     */
    template <typename... Args_t>
    OptionalSymbol_t &emplace(Args_t &&...args) {
#ifndef __clang_analyzer__
        // the emplace() is expected to be used only for uninitialized symbols
        if (initialized) throw std::runtime_error(__PRETTY_FUNCTION__);
#endif /* __clang_analyzer__ */
        new (&value) ValueType_t(std::forward<Args_t>(args)...);
        initialized = true;
        return *this;
    }

    /** D'tor.
     */
    ~OptionalSymbol_t() {if (initialized) value.~ValueType_t();}

    /** Returns constant reference to the held symbol value.
     */
    const ValueType_t &operator*() const {return value;}

    /** Returns reference to the held symbol value.
     */
    ValueType_t &operator*() {return value;}

    /** Returns constant pointer to the held symbol value.
     */
    const ValueType_t *operator->() const {return &value;}

    /** Returns pointer to the held symbol value.
     */
    ValueType_t *operator->() {return &value;}

public:
    struct Void_t {};
    bool initialized;      //!< true if value is valid/initialized
    union {
        Void_t void_value; //!< the fake value
        ValueType_t value; //!< the value of symbol
    };
};

/** Represents value of syntactic symbols.
 * It is used for terminal and nonterminal symbols in bison grammer.
 */
class Symbol_t {
public:
    /** C'tor.
     */
    Symbol_t(): id(), pos(), symbol_view() {}

    /** C'tor.
     */
    Symbol_t(const Token_t &token)
        : id(token), pos(token.pos), symbol_view(token.view())
    {}

    /** C'tor.
     */
    Symbol_t(int id, Pos_t pos = {}, string_view_t symbol_view = {})
        : id(id), pos(pos), symbol_view(symbol_view)
    {}

    /** Returns string representation of the symbol id.
     */
    string_view_t token_name() const {return l2_token_name(id);}

    /** Returns the original source data from which has been symbol parsed.
     */
    const string_view_t &view() const {return symbol_view;}

    /** Converts string view representing symbol to std::string.
     */
    std::string str() const {return symbol_view.str();}

    int id;                    //!< the LEX2 token id that creates the symbol
    Pos_t pos;                 //!< source position of such token
    string_view_t symbol_view; //!< the view to source code of whole symbol
};

/** The symbol representing the scalar value (int, real or string).
 */
class Literal_t: public Symbol_t {
public:
    /** C'tor.
     */
    template <typename type_t>
    Literal_t(const Token_t &token, type_t &&value)
        : Symbol_t(token), value(std::forward<type_t>(value))
    {}

    /** C'tor.
     */
    template <typename type_t>
    Literal_t(const Literal_t &other, type_t &&value)
        : Symbol_t(other.id, other.pos, other.symbol_view),
          value(std::forward<type_t>(value))
    {}

    /** Converts the view to the real number.
     */
    static double extract_real(string_view_t str) {
        return strtod(str.data(), nullptr);
    }

    /** Converts the view to the integral number.
     */
    static IntType_t extract_int(string_view_t str, int base) {
        return strtol(str.data(), nullptr, base);
    }

    /** Converts the view to the string value.
     */
    static std::string extract_str(Context_t *ctx, const Token_t &token);

    Value_t value; //!< the literal value
};

struct VarOffset_t {
    explicit operator bool() const {
        return (frame < invalid_offset) && (frag < invalid_offset);
    }
    static constexpr auto invalid_offset = std::numeric_limits<uint16_t>::max();
    uint64_t frame = invalid_offset;  //!< the offset of frame in stack
    uint64_t frag = invalid_offset;   //!< the offset of frag in frame
};

/** The symbol representing the variables in various contexts.
 */
class Variable_t: public Symbol_t {
public:
    /** C'tor.
     */
    Variable_t(): Symbol_t(), ident() {}

    /** C'tor.
     */
    Variable_t(const Token_t &token)
        : Symbol_t(token),
          ident(token)
    {}

    /** C'tor.
     */
    Variable_t(const Token_t &token, Identifier_t &&ident)
        : Symbol_t(token),
          ident(std::move(ident))
    {}

    /** C'tor.
     */
    Variable_t(const Variable_t &other, Identifier_t &&ident)
        : Symbol_t(other.id, other.pos, other.symbol_view),
          ident(std::move(ident))
    {}

    /** Appends new segment of variable identifier at the end of ident list.
     */
    Variable_t &push_back(const Token_t &token) {
        ident.push_back(token);
        symbol_view = {symbol_view.begin(), token.token_view.end()};
        id = token.token_id;
        return *this;
    }

    /** Removes the most recent segment of variable identifier.
     */
    Variable_t &pop_back(Context_t *ctx, const Pos_t &pos);

    VarOffset_t offset; //!< the offset of variable in stack of open frags
    Identifier_t ident; //!< the variable identifier
};

/** The symbol representing the directive options.
 */
class Options_t: public Symbol_t {
public:
    // types
    struct Option_t;
    using iterator = std::vector<Option_t>::iterator;
    using const_iterator = std::vector<Option_t>::const_iterator;

    /** C'tor.
     */
    Options_t(): Symbol_t() {}

    /** C'tor.
     */
    Options_t(const Token_t &token)
        : Symbol_t(token)
    {}

    /** Inserts new option to options.
     */
    void emplace(const string_view_t &ident, Value_t &&value) {
        auto ientry = find(ident);
        if (ientry == options.end())
            options.push_back({ident, std::move(value)});
        else ientry->value = std::move(value);
    }

    /** Returns iterator to the option with desired identifier or end().
     */
    iterator find(const string_view_t &ident) {
        return std::find(options.begin(), options.end(), ident);
    }

    /** Returns iterator to the option with desired identifier or end().
     */
    const_iterator find(const string_view_t &ident) const {
        return std::find(options.begin(), options.end(), ident);
    }

    /** Returns iterator to the first option.
     */
    iterator begin() {return options.begin();}

    /** Returns iterator one past the last option.
     */
    iterator end() {return options.end();}

    /** Returns iterator to the first option.
     */
    const_iterator begin() const {return options.begin();}

    /** Returns iterator one past the last option.
     */
    const_iterator end() const {return options.end();}

    /** Returns true if options are empty.
     */
    bool empty() const {return options.empty();}

    /** Purges options.
     */
    void clear() {options.clear();}

    /** The option identifier and value pair.
     */
    struct Option_t {
        string_view_t ident; //!< the option identifier
        Value_t value;       //!< the option value
        bool operator==(const string_view_t &rhs) const {return ident == rhs;}
    };

    std::vector<Option_t> options; //!< the options storage
};

/** The symbol used to transfer arity of expression with variadic argumets to
 * expression optimizer that needs the number.
 */
class NAryExpr_t: public Symbol_t {
public:
    /** C'tor.
     */
    NAryExpr_t(const Token_t &token, uint32_t arity, bool lazy_evaluated = 0)
        : Symbol_t(token),
          arity(arity), lazy_evaluated(lazy_evaluated)
    {}

    uint32_t arity;      //!< the arity of expression (the number of args)
    bool lazy_evaluated; //!< true if expression is lazy evaluated (||, &&, ...)
};

} // namespace Parser
} // namespace Teng

#endif /* TENGYYSTYPE_H */

