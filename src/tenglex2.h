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
 *
 * HISTORY
 * 2012-04-16  (volejnik)
 *             First version.
 */

#ifndef TENGLEX2_H
#define TENGLEX2_H

#include "tengsyntax.h"

/** @short Maximal depth of lex stack.
 */
static const unsigned int MAX_LEX_STACK_DEPTH = 50;

namespace Teng {

// Forward decl.
class ParserValue_t;

class Lex2_t {
public:
    Lex2_t();

    ~Lex2_t();

    /** @short Create new lex buffer for given string and push it onto the
     *         lex stack.
     * @return 0 OK, !0 error
     */
    int init(const string &src);

    int getElement(YYSTYPE *yylval_param,
                   ParserValue_t &value,
                   Error_t::Position_t &bufferPos,
                   Error_t &err);

    /** @short Destroy buffer at the top of lex stack.
     *  @return 0 OK, !0 error
     */
    int finish();

    /** @short Top element in the lex stack */
    unsigned int stackTop;

    /** @short Stack of lex buffers. */
    void * bufferStack[MAX_LEX_STACK_DEPTH];

    /** @short reentrant scanner instance */
    void * yyscanner;

private:
    Lex2_t(const Lex2_t &);
    Lex2_t &operator=(const Lex2_t &);
};

} // namespace Teng

#endif // TENGLEX2_H
