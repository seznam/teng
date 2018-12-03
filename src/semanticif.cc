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

#include <string>

#include "syntax.hh"
#include "semanticif.h"

#ifdef DEBUG
#define DBG(...) __VA_ARGS__
#else /* DEBUG */
#define DBG(...)
#endif /* DEBUG */

namespace Teng {
namespace Parser {
namespace {

/** Calculates if expression jump and updates appropriate instructions in
 * program.
 */
void finalize_if_branch(Context_t *ctx, int32_t shift) {
    // TODO(burlog): optimalize if

    // calculate real jump address for last elif/else branch
    auto branch_addr = ctx->curr_branch_addrs().pop();
    switch ((*ctx->program)[branch_addr].opcode()) {
    case OPCODE::JMP: {        // else branch
        auto &instr = (*ctx->program)[branch_addr].as<Jmp_t>();
        instr.addr_offset = ctx->program->size() - branch_addr - shift;
        break;
    }
    case OPCODE::JMP_IF_NOT: { // elif branch
        auto &instr = (*ctx->program)[branch_addr].as<JmpIfNot_t>();
        instr.addr_offset = ctx->program->size() - branch_addr - shift;
        break;
    }
    default:
        throw std::runtime_error(__PRETTY_FUNCTION__);
    }
}

} // namespace

void prepare_if_stmnt(Context_t *ctx, const Pos_t &pos) {
    // @note new values are pushed to the stack of branch addresses and
    // stack of the if statement starts. They have to be removed at the end of
    // if statement processing unless strange things can happen.
    // The finalize_if_stmnt does it for us.
    auto prgsize = static_cast<int64_t>(ctx->program->size());
    ctx->branch_addrs.push();
    ctx->if_start_points.push({pos, prgsize, true});
}

void finalize_if_stmnt(Context_t *ctx) {
    // @see prepare_if_stmnt note
    ctx->branch_addrs.pop();
    ctx->if_start_points.pop();
}

void generate_if(Context_t *ctx, const Token_t &token, bool valid_expr) {
    // generate if condition
    ctx->curr_branch_addrs().push(ctx->program->size());
    generate<JmpIfNot_t>(ctx, token.pos);

    // warn about invalid expression
    if (!valid_expr) {
        auto msg_end_if = "You forgot write condition of the if statement";
        auto msg_end_el = "You forgot write condition of the elif statement";
        auto msg_def_if = "Invalid expression in the if statement condition";
        auto msg_def_el = "Invalid expression in the elif statement condition";
        switch (ctx->unexpected_token) {
        case LEX2::END:
            logDiag(ctx, token.pos, token == LEX2::IF? msg_end_if: msg_end_el);
            break;
        default:
            logDiag(ctx, token.pos, token == LEX2::IF? msg_def_if: msg_def_el);
            break;
        }
        reset_error(ctx);
    }
}

void generate_if(Context_t *ctx, const Token_t &token, const Token_t &inv) {
    note_error(ctx, inv);
    generate_if(ctx, token, false);
}

void generate_endif(Context_t *ctx, const Pos_t *inv_pos) {
    // update if/else condition jump
    finalize_if_branch(ctx, 1);

    // now is known the endif address so we can update if brachnes end jumps
    while (!ctx->curr_branch_addrs().empty()) {
        auto branch_addr = ctx->curr_branch_addrs().pop();
        auto &instr = (*ctx->program)[branch_addr].as<Jmp_t>();
        instr.addr_offset = ctx->program->size() - branch_addr - 1;
    }

    // break possible invalid print optimization
    generate<Noop_t>(ctx);

    // warn if there is invalid tokens
    if (inv_pos) {
        logWarning(
            ctx,
            *inv_pos,
            "Ignoring invalid excessive tokens in <?teng endif?> directive"
        );
        reset_error(ctx);
    }
}

void generate_else(Context_t *ctx, const Token_t &token, bool invalid) {
    // update if/else condition jump
    finalize_if_branch(ctx, 0);

    // insert branch end jump
    ctx->curr_branch_addrs().push(ctx->program->size());
    generate<Jmp_t>(ctx, token.pos);

    // warn if there is invalid tokens
    if (invalid) {
        logWarning(
            ctx,
            token.pos,
            "Ignoring invalid excessive tokens in <?teng else?> directive"
        );
        reset_error(ctx);
    }
}

void prepare_elif(Context_t *ctx, const Token_t &token) {
    finalize_if_branch(ctx, 0);
    ctx->curr_branch_addrs().push(ctx->program->size());
    generate<Jmp_t>(ctx, token.pos);
}

void finalize_inv_if_stmnt(Context_t *ctx, const Token_t &token) {
    auto &pos = ctx->curr_if_start_point().pos;
    switch (token) {
    case LEX2::END:
    case LEX2_EOF:
        logError(
            ctx,
            pos,
            "Missing <?teng endif?> closing directive of <?teng if?> "
            "statement; discarding whole if statement"
        );
        break;
    case LEX2::ENDFRAGMENT:
        logError(
            ctx,
            pos,
            "The <?teng if?> block crosses the parent fragment block ending at="
            + token.pos.str() + "; discarding whole if statement"
        );
        break;
    case LEX2::ENDFORMAT:
        logError(
            ctx,
            pos,
            "The <?teng if?> block crosses the parent format block ending at="
            + token.pos.str() + "; discarding whole if statement"
        );
        break;
    case LEX2::ENDCTYPE:
        logError(
            ctx,
            pos,
            "The <?teng if?> block crosses the parent ctype block ending at="
            + token.pos.str() + "; discarding whole if statement"
        );
        break;
    default:
        logError(
            ctx,
            pos,
            "Error in <?teng if?> statement true branch; "
            "discarding whole if statement"
        );
        break;
    }
    ctx->program->erase_from(ctx->curr_if_start_point().addr);
    finalize_if_stmnt(ctx);
    note_error(ctx, token);
    reset_error(ctx);
}

void discard_if_stmnt(Context_t *ctx) {
    logError(
        ctx,
        ctx->curr_if_start_point().pos,
        "Disordered elif/else branches in <?teng if?> statement; "
        "discarding whole if statement"
    );
    ctx->program->erase_from(ctx->curr_if_start_point().addr);
    finalize_if_stmnt(ctx);
}

} // namespace Parser
} // namespace Teng

