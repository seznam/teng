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
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz
 *
 * HISTORY
 * 2018-07-07  (burlog)
 *             Adapted from tengsyntax.yy
 */

// TODO(burlog): remove it?
#include <iomanip>

#include "tengyystype.h"
#include "tengsyntax.hh"
#include "tenglogging.h"
#include "tengconfiguration.h"
#include "tengparsercontext.h"
#include "tengyylex.h"

namespace Teng {
namespace Parser {

int yylex(Symbol_t *symbol, Context_t *ctx) {
    Pos_t last_valid_pos;

    while (!ctx->lex1_stack.empty()) {
        // if level 2 lexer is currently in use get next L2 token and process it
        if (ctx->lex2.in_use()) {
            switch ((*symbol = ctx->lex2.next()).id) {
            default:
                std::cerr << "...." << "symbol=" << symbol->name()
                          << ", str=\"" << symbol->symbol_string << "\""
                          << std::endl;
                // symbol->prgsize = ctx->program->size();
                return symbol->id;
            case LEX2::INVALID:
                std::cerr << "...." << "invalid" << std::endl;
                logError(ctx, symbol->pos, "Invalid lexical element");
                continue;
            case 0: // means EOF
                std::cerr << "...." << "eof" << std::endl;
                ctx->lex2.finish_scanning();
                break;
            }
        }

        // get next L1 token and process it
        using LEX1 = Lex1_t::LEX1;
        bool accept_short_directives = ctx->params->isShortTagEnabled();
        std::cerr << "******************************************" << std::endl;
        switch (auto token = ctx->lex1().next(accept_short_directives)) {
        case LEX1::TENG: case LEX1::EXPR:
        case LEX1::DICT: case LEX1::TENG_SHORT:
            std::cerr << "* teng,expr,dict" << " " << token.flex_view() << std::endl;
            ctx->lex2.start_scanning(std::move(token.flex_view()), token.pos);
            continue; // parse token value by level 2 lexer in next loop

        case LEX1::TEXT:
            std::cerr << "* text" << std::endl;
            symbol->id = LEX2::TEXT;
            symbol->symbol_string = token.string_view();
            symbol->pos = token.pos;
            // symbol->prgsize = ctx->program->size();
            return symbol->id;

        case LEX1::ERROR:
            std::cerr << "* error" << std::endl;
            logError(ctx, token.pos, token.string_view());
            continue; // ignore bad element

        case LEX1::END_OF_INPUT:
            std::cerr << "* eof" << std::endl;
            // EOF from current lex so remove it from stack and try again
            last_valid_pos = ctx->lex1().position();
            ctx->lex1_stack.pop();
            break;
        }
    }

    // stop if there is nothing to do
    std::cerr << "******************************************" << std::endl;
    symbol->id = 0;
    symbol->symbol_string = {};
    symbol->pos = last_valid_pos;
    // symbol->prgsize = ctx->program->size();
    return 0; // EOF
}

} // namespace Parser
} // namespace Teng

