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
 * Michal Bukovsky
 *
 * HISTORY
 * 2018-05-07  (burlog)
 *             Adapted from tengsyntax.yy
 */

#include "tengyystype.h"
#include "tengparsercontext.h"

namespace Teng {

int yylex(LeftValue_t *leftValue, ParserContext_t *ctx) {
    for (;;) {
        // if lex2 is currently in use
        if (ctx->lex2InUse) {
            ParserValue_t val;
            Error_t::Position_t pos = ctx->lex2Pos;

            // get next L2 token and process it
            switch (auto token = ctx->lex2.getElement(leftValue, val, ctx)) {
            default:
                leftValue->val = val;
                leftValue->pos = pos; // elements position
                ctx->position = pos; // actual position in input stream
                leftValue->prgsize = ctx->program->size();
                return token;
            case Parser::parser::token::LEX_INVALID:
                ctx->logError(pos, "Invalid lexical element");
                continue;
            case 0:
                // that was last token
                ctx->lex2.finish();
                ctx->lex2InUse = false;
                break;
            }
        }

        // test if lex1 stack is empty
        if (ctx->lex1.empty()) {
            leftValue->pos = ctx->lex1Pos; // last known position
            ctx->position = ctx->lex1Pos; // last known position in input stream
            leftValue->prgsize = ctx->program->size();
            return 0; // EOF
        }

        // remember the token position
        ctx->lex1Pos = ctx->lex1.top()->getPosition();
        bool shortTag = ctx->paramDictionary->isShortTagEnabled();

        // get next L1 token and process it
        auto token = ctx->lex1.top()->getElement(shortTag);
        switch (token.type) {
        case Lex1_t::TYPE_TENG:
        case Lex1_t::TYPE_EXPR:
        case Lex1_t::TYPE_DICT:
        expr_label:
            // use lex2
            ctx->lex2.init(token.value);
            ctx->lex2Pos = ctx->lex1Pos;
            ctx->lex2InUse = true;
            continue; // read first lex2-element in next loop

        case Lex1_t::TYPE_TEXT:
        text_label:
            // plain text
            leftValue->val.setString(token.value);
            leftValue->pos = ctx->lex1Pos; // element's position
            ctx->position = ctx->lex1Pos;
            leftValue->prgsize = ctx->program->size();
            return Parser::parser::token::LEX_TEXT;

        case Lex1_t::TYPE_TENG_SHORT:
            if (shortTag) goto expr_label;
            else goto text_label;

        case Lex1_t::TYPE_ERROR:
            // error
            ctx->logError(ctx->lex1Pos, token.value);
            continue; // ignore bad element

        case Lex1_t::TYPE_EOF:
            // EOF from current lex
            // remove it from stack and try again
            delete ctx->lex1.top();
            ctx->lex1.pop();
            ctx->sourceIndex.pop();
            break;
        }
    }
}

} // namespace Teng

