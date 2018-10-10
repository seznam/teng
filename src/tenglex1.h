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

// forwards
class Configuration_t;

namespace Parser {

/** First level of lexical analyzer that generaly splits input to text tokens
 * that does not contain teng expressions and to tokens candidating to valid
 * teng expressions. The teng expressions are analyzed by second level lexical
 * analyzer. See Parser::Context_t::next_token() function where the magic of
 * two analyzers is hidden.
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
        ESC_EXPR,     //!< Shorted expression form
        RAW_EXPR,     //!< Shorted expression form
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
            case LEX1::TEXT:
            case LEX1::END_OF_INPUT: case LEX1::ERROR:
                new (&string_view_value)
                    string_view_t(std::move(token.string_view_value));
                return;
            case LEX1::DICT:
            case LEX1::TENG: case LEX1::TENG_SHORT:
            case LEX1::ESC_EXPR: case LEX1::RAW_EXPR:
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
            case LEX1::TEXT:
            case LEX1::END_OF_INPUT: case LEX1::ERROR:
                string_view_value.~string_view_t();
                return;
            case LEX1::DICT:
            case LEX1::TENG: case LEX1::TENG_SHORT:
            case LEX1::ESC_EXPR: case LEX1::RAW_EXPR:
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
            case LEX1::TEXT:
            case LEX1::END_OF_INPUT: case LEX1::ERROR:
                return string_view_value;
            case LEX1::DICT:
            case LEX1::TENG: case LEX1::TENG_SHORT:
            case LEX1::ESC_EXPR: case LEX1::RAW_EXPR:
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
    Lex1_t(
        flex_string_value_t &source_code,
        bool utf8,
        const Configuration_t *params,
        const std::string *filename = {}
    ): source_code(source_code), offset(0), pos(filename, 1, 0), utf8(utf8),
       params(params)
    {}

    /** C'tor: move.
     */
    Lex1_t(Lex1_t &&) = default;

    /** Assigment: move.
     */
    Lex1_t &operator=(Lex1_t &&) = default;

    /** Returns the next level 1 token.
     */
    Token_t next();

    /** Get position within the input string.
     */
    const Pos_t &position() const {return pos;}

private:
    // don't copy
    Lex1_t(const Lex1_t &) = delete;
    Lex1_t &operator=(const Lex1_t &) = delete;

    /** The state of the lexer.
     */
    enum class state {
        initial,
        end_of_input,
        long_directive,
        short_directive,
        esc_expr_directive,
        raw_expr_directive,
        dict_directive,
        comment_directive,
    } current_state = state::initial;

    flex_string_view_t source_code; //!< source code passed on init
    std::size_t offset;             //!< position within the source code
    Pos_t pos;                      //!< position within the "page"
    bool utf8;                      //!< if pos should be in utf8 chars
    const Configuration_t *params;  //!< teng configuration
};

/** Writes token description to the stream.
 */
std::ostream &operator<<(std::ostream &os, const Lex1_t::Token_t &token);

} // namespace Parser
} // namespace Teng

#endif /* TENGLEX1_H */


