/* !don't remove! -*- C++ -*-
 *
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
 * $Id: tengsyntax.yy,v 1.14 2010-06-11 08:25:35 burlog Exp $
 *
 * DESCRIPTION
 * Teng grammar semantic actions.
 *
 * AUTHORS
 * Stepan Skrob <stepan@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-06-07  (burlog)
 *             Moved from syntax.yy.
 */

#include <cstdio>
#include <string>

#include "lex2.h"
#include "debug.h"
#include "program.h"
#include "instruction.h"
#include "parsercontext.h"
#include "semantic.h"

namespace Teng {
namespace Parser {
namespace {

} // namespace

Token_t note_error(Context_t *ctx, const Token_t &token) {
    if (!ctx->error_occurred) {
        ctx->error_occurred = true;
        ctx->unexpected_token = token;
        ExprDiag_t::log_unexpected_token(ctx);
    }
    return token;
}

void reset_error(Context_t *ctx) {
    ctx->error_occurred = false;
}

void expr_diag(Context_t *ctx, diag_code_type new_diag_code, bool pop) {
    if (pop) ctx->expr_diag.pop();
    ctx->expr_diag.push({new_diag_code, ctx->pos()});
}

void expr_diag_sentinel(Context_t *ctx, diag_code new_diag_code) {
    ctx->expr_diag.push_sentinel();
    expr_diag(ctx, new_diag_code, false);
}

void generate_val(Context_t *ctx, const Pos_t &pos, Value_t value) {
    generate<Val_t>(ctx, std::move(value), pos);
}

} // namespace Parser
} // namespace Teng

