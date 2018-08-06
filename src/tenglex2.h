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
 * String view.
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

#include "tengposition.h"
#include "tengyystype.h"
#include "tenglex1.h"

namespace Teng {
namespace Parser {

/** This is glue class between lexer and parser of teng directives like
 * <?teng ...?> are.
 */
class Lex2_t {
public:
    /** C'tor.
     */
    Lex2_t(Error_t &err);

    /** D'tor.
     */
    ~Lex2_t();

    /** Create new lexer buffer for given string.
     */
    void
    start_scanning(flex_string_view_t &&new_directive, const Pos_t &init_pos);

    /** Entry point of lexer that do all the work.
     */
    Symbol_t next();

    /** Destroy buffer at the top of lexer stack.
     */
    void finish_scanning();

    /** Returns true if level 2 lexer is in use.
     */
    bool in_use() const {return buffer;}

    /** Returns current "page" position within source code.
     */
    const Pos_t &position() const {return pos;}

    /** Saves start position of the symbol.
     */
    void make_symbol_start(const char *ipos) {
        symbol_pos = pos;
        symbol_ipos = ipos;
    }

    /** Creates new symbol begins at symbol_ipos and ends at epos.
     */
    Symbol_t make_symbol(int id, const char *epos) {
        pos.advance(symbol_ipos, epos);
        return {symbol_pos, id, {symbol_ipos, epos}};
    }

    /** Creates new symbol begins at ipos and ends at epos.
     */
    Symbol_t make_symbol(int id, const char *ipos, const char *epos) {
        make_symbol_start(ipos);
        return make_symbol(id, epos);
    }

    /** Creates new symbol begins at ipos and ends at epos. It assumes that
     * characters within <ipos, epos) range contain no newlines.
     */
    Symbol_t make_oneline_symbol(int id, const char *ipos, const char *epos) {
        symbol_pos = pos;
        pos.advanceColumn(epos - ipos);
        return {symbol_pos, id, {ipos, epos}};
    }

protected:
    // don't copy
    Lex2_t(const Lex2_t &) = delete;
    Lex2_t &operator=(const Lex2_t &) = delete;

    void *yyscanner;              //!< reentrant flex lexer instance
    void *buffer;                 //!< buffer for flex lexer
    Error_t &err;                 //!< error log
    Pos_t pos;                    //!< current pos on "page"
    Pos_t symbol_pos;             //!< start pos of not yet fully parsed symbol
    const char *symbol_ipos;      //!< ptr to first char of not yet fully ...
    flex_string_view_t directive; //!< the teng directive source code
};

} // namespace
} // namespace Teng

#endif // TENGLEX2_H

