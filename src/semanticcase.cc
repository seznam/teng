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

#include "debug.h"
#include "program.h"
#include "logging.h"
#include "parserdiag.h"
#include "instruction.h"
#include "parsercontext.h"
#include "semanticcase.h"

namespace Teng {
namespace Parser {

void prepare_case(Context_t *ctx) {
    expr_diag_sentinel(ctx, diag_code::case_cond);
    ctx->case_option_addrs.push();
}

void prepare_case_cond(Context_t *ctx, const Token_t &token) {
    expr_diag(ctx, diag_code::case_option);
    generate<PrgStackPush_t>(ctx, token.pos);
}

uint32_t generate_case_cmp(Context_t *ctx, Literal_t &literal) {
    // warn if there is more than one branch valid for same value
    for (auto addr: ctx->case_option_addrs.top()) {
        auto &instr = (*ctx->program)[addr].as<Val_t>();
        if (instr.value == literal.value) {
            auto val = literal.value.string();
            logWarning(ctx, instr.pos(), "Duplicit case operand: " + val);
            logWarning(ctx, literal.pos, "Next seen here");
            break;
        }
    }

    // generate instructions
    generate<PrgStackAt_t>(ctx, 0, literal.pos);
    ctx->case_option_addrs.top().push(ctx->program->size());
    generate<Val_t>(ctx, std::move(literal.value), literal.pos);
    generate<EQ_t>(ctx, literal.pos);
    return 0;
}

void update_case_jmp(Context_t *ctx, const Token_t &token, uint32_t alts) {
    expr_diag(ctx, diag_code::case_option_branch);
    for (; alts; --alts) {
        auto addr = ctx->curr_branch_addrs().pop();
        auto &instr = (*ctx->program)[addr].as<Or_t>();
        instr.addr_offset = ctx->program->size() - addr - 1;
    }
    ctx->curr_branch_addrs().push(ctx->program->size());
    generate<JmpIfNot_t>(ctx, token.pos);
}

uint32_t
generate_case_next(Context_t *ctx, Literal_t &literal, uint32_t alts) {
    ctx->curr_branch_addrs().push(ctx->program->size());
    generate<Or_t>(ctx, literal.pos);
    generate_case_cmp(ctx, literal);
    return alts + 1;
}

void finalize_case_branch(Context_t *ctx, const Token_t &token) {
    auto branch_case_addr = ctx->curr_branch_addrs().pop();
    auto &instr = (*ctx->program)[branch_case_addr].as<JmpIfNot_t>();
    instr.addr_offset = ctx->program->size() - branch_case_addr;
    ctx->curr_branch_addrs().push(ctx->program->size());
    generate<Jmp_t>(ctx, token.pos);
}

NAryExpr_t finalize_case(Context_t *ctx, const Token_t &token, uint32_t arity) {
    // update addr offsets of jmp instruction at the end of all branches
    for (auto tmp_arity = arity; --tmp_arity;) {
        auto branch_end_addr = ctx->curr_branch_addrs().pop();
        auto &instr = (*ctx->program)[branch_end_addr].as<Jmp_t>();
        instr.addr_offset = ctx->program->size() - branch_end_addr - 1;
    }

    // generate instruction that remove case value from prg stack
    generate<PrgStackPop_t>(ctx, token.pos);

    // clear stored option addresses
    ctx->case_option_addrs.pop();

    // clear case expr diagnostic code
    ctx->expr_diag.pop();

    // done
    return NAryExpr_t(token, arity + 1, true);
}

} // namespace Parser
} // namespace Teng

