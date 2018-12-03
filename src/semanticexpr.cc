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

#include "regex.h"
#include "logging.h"
#include "program.h"
#include "instruction.h"
#include "parsercontext.h"
#include "semanticexpr.h"

#ifdef DEBUG
#include <iostream>
#define DBG(...) __VA_ARGS__
#else /* DEBUG */
#define DBG(...)
#endif /* DEBUG */

namespace Teng {
namespace Parser {
namespace {

/** Generates binary operator (OR, AND, ...).
 */
template <typename Instr_t>
void generate_bin_op(Context_t *ctx, const Token_t &token) {
    ctx->branch_addrs.top().push(ctx->program->size());
    generate<Instr_t>(ctx, token.pos);
}

/** Finalizes binary operator (OR, AND, ...).
 */
template <typename Instr_t>
void finalize_bin_op(Context_t *ctx) {
    auto bin_op_addr = ctx->branch_addrs.top().pop();
    auto addr_offset = ctx->program->size() - bin_op_addr - 1;
    (*ctx->program)[bin_op_addr].as<Instr_t>().addr_offset = addr_offset;

    // breaks invalid print optimization
    generate<Noop_t>(ctx);
}

} // namespace

void note_expr_start_point(Context_t *ctx, const Pos_t &pos) {
    if (ctx->expr_start_point.update_allowed) {
        ctx->expr_start_point = {
            pos,
            static_cast<int64_t>(ctx->program->size()),
            false
        };
    }
}

void note_optimization_point(Context_t *ctx, bool optimizable) {
    int64_t addr = ctx->program->size() - 1;
    ctx->optimization_points.push({addr, optimizable});
}

void optimize_expr(Context_t *ctx, uint32_t arity, bool lazy_evaluated) {
    // take the address of the first expression argument instruction
    bool optimizable = true;
    auto args_point = ctx->program->size() - 1;
    if (arity) {
        for (; --arity; ctx->optimization_points.pop())
            optimizable &= ctx->optimization_points.top().optimizable;
        args_point = ctx->optimization_points.top().addr;
        optimizable = (optimizable || lazy_evaluated)
                   && ctx->optimization_points.top().optimizable;
        ctx->optimization_points.pop();
    }

    // if the args are not optimizable, the expression itself is not optimizable
    if (optimizable) {
        // try to evaluate given part of program
        Value_t result = ctx->coproc.eval(&ctx->open_frames, args_point);
        if (!result.is_undefined()) {
            // remove expression's program and replace it with its value
            DBG(std::cerr << "$$$$ optimized => " << result << std::endl);
            auto pos = (*ctx->program)[args_point].pos();
            ctx->program->erase_from(args_point);
            generate_val(ctx, pos, std::move(result));

        } else optimizable = false;
        DBG(if (!optimizable) std::cerr << "$$$$ can't optimize" << std::endl);

    } else {DBG(std::cerr << "$$$$ unoptimizable" << std::endl);}

    // each expression has a value that plays its role in further optimization
    // if optimization fails then it is ok to replace it with one imaginary
    // value at current address because it is marked as unoptimizable
    note_optimization_point(ctx, optimizable);
}

void discard_expr(Context_t *ctx) {
    // the note_expr_start_point() hasn't been called for that expression
    // because the first token is invalid so note this token position as
    // expression start
    // (the unexpected_token is always the first of an invalid token sequence)
    if (ctx->expr_start_point.update_allowed)
        note_expr_start_point(ctx, ctx->unexpected_token.pos);
    ctx->expr_start_point.update_allowed = true;

    // discard whole expression code and replace it with undefined
    ctx->program->erase_from(ctx->expr_start_point.addr);
    generate<Val_t>(ctx, Value_t(), ctx->expr_start_point.pos);

    // if there is diagnostics then process it
    ctx->expr_diag.unwind(ctx, ctx->unexpected_token);

    // tell the user that he should fix the expression
    logError(
        ctx,
        ctx->expr_start_point.pos,
        "Invalid expression, fix it please; "
        "replacing whole expression with undefined value"
    );

    // fix bin/tern/case operator's stack of addresses
    ctx->branch_addrs.pop();

    // also reset optimization points
    // (yes, it clears even nested points - we don't trust its because of error)
    ctx->optimization_points.clear();

    // reset error marker
    reset_error(ctx);
}

void finish_expr(Context_t *ctx) {
    ctx->expr_start_point.update_allowed = true;
    ctx->expr_diag.clear();
    ctx->branch_addrs.pop();
}

void prepare_expr(Context_t *ctx, const Pos_t &) {
    ctx->branch_addrs.push();
}

/** Generates expression from given symbol.
 */
uint32_t generate_str_expr(Context_t *ctx, const Token_t &token, bool negate) {
    // TODO(burlog): delete this when STR_EQ/STR_NE is over
    if (!ctx->program->empty()) {
        auto &instr = ctx->program->back();
        if (instr.opcode() == OPCODE::VAL) {
            auto &value = instr.as<Val_t>().value;
            if (value.is_regex()) {
                if (!value.as_regex().unique())
                    throw std::runtime_error(__PRETTY_FUNCTION__);
                auto regex = std::move(value.as_regex());
                ctx->program->pop_back();
                generate<MatchRegex_t>(ctx, std::move(regex), token.pos);
                ctx->optimization_points.pop();
                if (negate)
                    generate<Not_t>(ctx, token.pos);
                return 1;
            }
        }
    }
    if (negate) generate_expr<StrNE_t>(ctx, token);
    else  generate_expr<StrEQ_t>(ctx, token);
    return 2;
}

void generate_bin_and(Context_t *ctx, const Token_t &token) {
    generate_bin_op<And_t>(ctx, token);
}

void finalize_bin_and(Context_t *ctx) {
    finalize_bin_op<And_t>(ctx);
}

void generate_bin_or(Context_t *ctx, const Token_t &token) {
    generate_bin_op<Or_t>(ctx, token);
}

void finalize_bin_or(Context_t *ctx) {
    finalize_bin_op<Or_t>(ctx);
}

} // namespace Parser
} // namespace Teng

