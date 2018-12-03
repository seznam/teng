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

#include "program.h"
#include "parserdiag.h"
#include "instruction.h"
#include "parsercontext.h"
#include "semantictern.h"

#ifdef DEBUG
#include <iostream>
#define DBG(...) __VA_ARGS__
#else /* DEBUG */
#define DBG(...)
#endif /* DEBUG */

namespace Teng {
namespace Parser {

void generate_tern_op(Context_t *ctx, const Token_t &token) {
    ctx->curr_branch_addrs().push(ctx->program->size());
    generate<JmpIfNot_t>(ctx, token.pos);
    expr_diag(ctx, diag_code::tern_true_branch, false);
}

void finalize_tern_op_true_branch(Context_t *ctx, const Token_t &token) {
    // calculate jump from begin of tern op to false branch
    auto cond_addr = ctx->curr_branch_addrs().pop();
    auto false_branch_offset = ctx->program->size() - cond_addr;

    // store address of jump from true branch
    ctx->curr_branch_addrs().push(ctx->program->size());
    generate<Jmp_t>(ctx, token.pos);

    // fix conditional jump offset (relative addr)
    auto &instr = (*ctx->program)[cond_addr].as<JmpIfNot_t>();
    instr.addr_offset = false_branch_offset;

    // diagnostic code
    expr_diag(ctx, diag_code::tern_false_branch);
}

void finalize_tern_op_false_branch(Context_t *ctx) {
    auto true_branch_jump_addr = ctx->curr_branch_addrs().pop();
    auto tern_op_end_offset = ctx->program->size() - true_branch_jump_addr - 1;
    auto &instr = (*ctx->program)[true_branch_jump_addr].as<Jmp_t>();
    instr.addr_offset = tern_op_end_offset;
    ctx->expr_diag.pop();

    // breaks invalid print optimization
    generate<Noop_t>(ctx);
}

} // namespace Parser
} // namespace Teng

