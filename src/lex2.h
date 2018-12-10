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
 * Level 2 lexer.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz
 *
 * HISTORY
 * 2018-07-07  (burlog)
 *             First version.
 */

#ifndef TENGLEX2_H
#define TENGLEX2_H

#include <string>

#include "lex1.h"
#include "position.h"

namespace Teng {
namespace Parser {

// the eof token id
constexpr int LEX2_EOF = 0;

/** Converts token id to its string representation.
 */
string_view_t l2_token_name(int token_id);

/** Represents level 2 lexical token.
 */
class Token_t {
public:
    /** Converts token to its id.
     */
    operator int() const {return token_id;}

    /** Returns string representation of the token id.
     */
    string_view_t token_name() const {return l2_token_name(token_id);}

    /** Returns the original source data from which has been token parsed.
     */
    const string_view_t &view() const {return token_view;}

    /** Converts string view representing token to std::string.
     */
    std::string str() const {return token_view.str();}

    int token_id;             //!< the token id
    Pos_t pos;                //!< the position is source code
    string_view_t token_view; //!< raw string data of the token
};

/** Writes token description to the stream.
 */
std::ostream &operator<<(std::ostream &os, const Token_t &token);

/** This is glue class between lexer and parser of teng directives like
 * <?teng ...?> are.
 */
class Lex2_t {
public:
    /** C'tor.
     */
    Lex2_t(const Configuration_t *params, bool utf8, Error_t &err);

    /** D'tor.
     */
    ~Lex2_t();

    /** Create new lexer buffer for given string.
     */
    void
    start_scanning(
        flex_string_view_t &&new_directive,
        const Pos_t &init_pos
    );

    /** Switches the scanner to block_override start condition.
     */
    void switch_to_override_block();

    /** Switches the scanner to initial start condition.
     */
    void switch_to_initial();

    /** Entry point of lexer that do all the work.
     *
     * The implementation is provided by the flex tool.
     */
    Token_t next();

    /** Destroy buffer at the top of lexer stack.
     */
    void finish_scanning();

    /** Returns true if level 2 lexer is in use.
     */
    bool in_use() const {return buffer;}

    /** Returns current "page" position within source code.
     */
    const Pos_t &position() const {return pos;}

    /** Saves start position of the token.
     */
    void make_token_start(const char *ipos) {
        token_pos = pos;
        token_ipos = ipos;
    }

    /** Creates new token begins at token_ipos and ends at epos.
     */
    Token_t make_token(int id, const char *epos) {
        pos.advance(token_ipos, epos);
        return {id, token_pos, {token_ipos, epos}};
    }

    /** Creates new token begins at ipos and ends at epos.
     */
    Token_t make_token(int id, const char *ipos, const char *epos) {
        make_token_start(ipos);
        return make_token(id, epos);
    }

    /** Creates new token begins at ipos and ends at epos. It assumes that
     * characters within <ipos, epos) range contain no newlines.
     */
    Token_t make_nonewline_token(int id, const char *ipos, const char *epos) {
        token_pos = pos;
        pos.advanceColumn(epos - ipos);
        return {id, token_pos, {ipos, epos}};
    }

protected:
    // don't copy
    Lex2_t(const Lex2_t &) = delete;
    Lex2_t &operator=(const Lex2_t &) = delete;

    /** Sets start condition after scanner initialization.
     */
    void set_start_condition(int start_condition);

    void *yyscanner;               //!< reentrant flex lexer instance
    void *buffer;                  //!< buffer for flex lexer
    Error_t &err;                  //!< error log
    const Configuration_t *params; //!< configuration
    bool utf8;                     //!< true if template is in utf8 encoding
    Pos_t pos;                     //!< current pos on "page"
    Pos_t token_pos;               //!< start pos of not yet fully parsed token
    const char *token_ipos;        //!< ptr to first char of not yet fully ...
    flex_string_view_t directive;  //!< the teng directive source code
    int current_start_condition;   //!< what start condition is in use
};

} // namespace Parser
} // namespace Teng

#endif // TENGLEX2_H

