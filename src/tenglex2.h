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

#include <string>

#include "tengerror.h"
#include "tengyystype.h"
#include "tengsyntax.hh"


namespace Teng {

// Forward decl.
class ParserValue_t;

/**
 * @short Maximal depth of lex stack.
 */
static const unsigned int MAX_LEX_STACK_DEPTH = 50;

/**
 * @short This is glue class between lexer and parser.
 */
class Lex2_t {
public:
    // don't copy
    Lex2_t(const Lex2_t &) = delete;
    Lex2_t &operator=(const Lex2_t &) = delete;

    /**
     * @short C'tor.
     */
    Lex2_t();

    /**
     * @short D'tor.
     */
    ~Lex2_t();

    /**
     * @short Create new lex buffer for given string and push it onto the
     * lex stack.
     *
     * @return 0 OK, !0 error
     */
    int init(const std::string &src);

    /**
     * @short Entry point of lexer that do all the work.
     */
    int getElement(LeftValue_t *yylval_param,
                   ParserValue_t &value,
                   Error_t::Position_t &bufferPos,
                   Error_t &err);

    template <typename Ctx_t>
    int getElement(LeftValue_t *yylval, ParserValue_t &value, Ctx_t *ctx) {
        return getElement(yylval, value,
                          ctx->lex2Pos, ctx->program->getErrors());
    }

    /**
     * @short Destroy buffer at the top of lex stack.
     *
     *  @return 0 OK, !0 error
     */
    int finish();

    unsigned int stackTop;                  //!< top element in the lex stack
    void *bufferStack[MAX_LEX_STACK_DEPTH]; //!< stack of lex buffers
    void *yyscanner;                        //!< reentrant scanner instance
};

} // namespace Teng

#endif // TENGLEX2_H

