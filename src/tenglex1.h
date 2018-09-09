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
 * Teng level #1 lexical analyzer.
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

#ifndef TENGLEX1_H
#define TENGLEX1_H

#include <string>

#include "tengposition.h"
#include "tengflexhelpers.h"

namespace Teng {
namespace Parser {

/** First level of lexical analyzer that generaly splits input to text tokens
 * that does not contain teng expressions and to tokens candidating to valid
 * teng expressions. The teng expressions are analyzed by second level lexical
 * analyzer. See yylex() function where the magic of two analyzers is hidden.
 */
class Lex1_t {
public:
    /** Possible token types. */
    enum class LEX1 {
        END_OF_INPUT, //!< End of input, last token
        ERROR,        //!< Parse error, token.value contains error message
        TEXT,         //!< General text
        TENG,         //!< Teng directive
        TENG_SHORT,   //!< Teng short directive
        EXPR,         //!< Shorted expression form
        DICT,         //!< Shorted dictionary item form
    };

    /** Token type, returned by the next() method.
     */
    struct Token_t {
    public:
        /** C'tor: init of teng, teng_short, expr and dict tokens.
        */
        Token_t(LEX1 token_id, const Pos_t &pos, flex_string_view_t view)
            : token_id(token_id), pos(pos), flex_view_value{std::move(view)}
        {}

        /** C'tor: init of error, text and eoi tokens.
        */
        Token_t(LEX1 token_id, const Pos_t &pos, string_view_t view)
            : token_id(token_id), pos(pos), string_view_value{view}
        {}

        /** C'tor: move.
         */
        Token_t(Token_t &&token)
            : token_id(token.token_id), pos(token.pos)
        {
            switch (token_id) {
            case LEX1::END_OF_INPUT: case LEX1::ERROR:
            case LEX1::TEXT:
                new (&string_view_value)
                    string_view_t(std::move(token.string_view_value));
                return;
            case LEX1::TENG: case LEX1::TENG_SHORT:
            case LEX1::EXPR: case LEX1::DICT:
                new (&flex_view_value)
                    flex_string_view_t(std::move(token.flex_view_value));
                return;
            }
        }

        /** Assigment: move.
         */
        Token_t &operator=(Token_t &&other) {
            if (this != &other) {
                this->~Token_t();
                new (this) Token_t(std::move(other));
            }
            return *this;
        }

        /** D'tor.
         */
        ~Token_t() {
            switch (token_id) {
            case LEX1::END_OF_INPUT: case LEX1::ERROR:
            case LEX1::TEXT:
                string_view_value.~string_view_t();
                return;
            case LEX1::TENG: case LEX1::TENG_SHORT:
            case LEX1::EXPR: case LEX1::DICT:
                flex_view_value.~flex_string_view_t();
                return;
            }
        }

        /** Returns token id.
         */
        operator LEX1() const {return token_id;}

        /** Returns the original source data from which has been token parsed.
        */
        string_view_t view() const {
            switch (token_id) {
            case LEX1::END_OF_INPUT: case LEX1::ERROR:
            case LEX1::TEXT:
                return string_view_value;
            case LEX1::TENG: case LEX1::TENG_SHORT:
            case LEX1::EXPR: case LEX1::DICT:
                return flex_view_value;
            }
        }

        /** Returns value for error, text and end-of-input tokens.
         */
        string_view_t &string_view() {return string_view_value;}

        /** Returns value for teng, teng_short, expr and dict tokens.
         */
        flex_string_view_t &flex_view() {return flex_view_value;}

        /** Returns string representation of the token id.
        */
        const char *name() const;

        LEX1 token_id; //!< token id
        Pos_t pos;     //!< position of the token in source

    protected:
        union {
            string_view_t string_view_value;
            flex_string_view_t flex_view_value;
        };
    };

    /** Initialize lexical analyzer from string.
     *
     * The second argument is pointer to filename that must live as long as
     * Lex1_t object.
     *
     * The level 1 lexer will change the source code buffer.
     *
     * @param source_code Input string.
     * @param filename Pointer to filename.
     */
    Lex1_t(flex_string_view_t source_code, const std::string *filename = {})
        : source_code(std::move(source_code)), offset(0), pos(filename, 1, 0)
    {}

    /** C'tor: move.
     */
    Lex1_t(Lex1_t &&) = default;

    /** Assigment: move.
     */
    Lex1_t &operator=(Lex1_t &&) = default;

    /** Get next token.
     *
     * @param accept_short_directive enable short tah
     *
     * @return Token struct of next token.
     */
    Token_t next(bool accept_short_directive);

    /** Get position within the input string.
     */
    const Pos_t &position() const {return pos;}

private:
    // don't copy
    Lex1_t(const Lex1_t &) = delete;
    Lex1_t &operator=(const Lex1_t &) = delete;

    flex_string_view_t source_code; //!< source code passed on init
    std::size_t offset;             //!< position within the source code
    Pos_t pos;                      //!< position within the "page"
};

/** Writes token description to the stream.
 */
std::ostream &operator<<(std::ostream &os, const Lex1_t::Token_t &token);

} // namespace Parser
} // namespace Teng

#endif /* TENGLEX1_H */


