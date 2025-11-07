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
#include "syntax.hh"
#include "logging.h"
#include "program.h"
#include "instruction.h"
#include "parsercontext.h"
#include "configuration.h"
#include "semanticvar.h"
#include "semanticfrag.h"

namespace Teng {
namespace Parser {
namespace {

/** Generates expression open frag instruction.
 */
void open_frag(Context_t *ctx, const Token_t &token, const Pos_t &pos) {
    (token == LEX2::BUILTIN_ERROR) && ctx->params->isErrorFragmentEnabled()
        ? generate<OpenErrorFrag_t>(ctx, pos)
        : generate<OpenFrag_t>(ctx, token.view().str(), pos);
}

/** Casts given instruction to OPEN_FRAG instruction.
 */
OpenFrag_t &open_frag_cast(Instruction_t &instr) {
    switch (instr.opcode()) {
    case OPCODE::OPEN_ERROR_FRAG:
        return instr.as<OpenErrorFrag_t>();
    case OPCODE::OPEN_FRAG:
        return instr.as<OpenFrag_t>();
    default:
        throw bad_instr_cast_t(instr.opcode(), OPCODE::OPEN_FRAG);
    }
}

} // namespace

void open_frag(Context_t *ctx, const Pos_t &pos, Variable_t &frag) {
    // if symbol is invalid then create frag instruction with empty name that
    // is used as marker for close_frag() function to discard frag content
    if (frag.ident.empty()) {
        logError(
            ctx,
            frag.pos,
            "Empty fragment identifier; discarding fragment block content"
        );
        ctx->open_frames.top().open_frag({}, ctx->program->size(), false);
        generate<OpenFrag_t>(ctx, std::string(), pos);
        return;
    }

    // for open_frag purposes local and absolute variable are sufficient
    // but the relative ones should be resolved before we open any new frag
    if (frag.ident.is_relative())
        if ((frag.offset = resolve_relative_var(ctx, frag)))
            frag.ident = make_absolute_ident(ctx, frag);

    // create new open-fragments frame if identifier is absolute
    auto i = 0lu;
    auto *frame = &ctx->open_frames.top();
    if (frag.ident.is_absolute()) {
        // left-trim frag ident == make it relative to current open fragment
        // or open new frame if absolute ident can't be made relative
        if (!frame->is_prefix_of(frag.ident)) {
            frame = &ctx->open_frames.open_frame();
            generate<OpenFrame_t>(ctx, pos);
        } else i = frame->size();
    }

    // open as many fragments as needed
    for (auto first_index = i; i < frag.ident.size(); ++i) {
        bool auto_close = i != first_index;
        frame->open_frag(frag.ident[i], ctx->program->size(), auto_close);
        open_frag(ctx, frag.ident[i], pos);
    }
}

void open_inv_frag(Context_t *ctx, const Pos_t &pos) {
    switch (ctx->unexpected_token) {
    case LEX2::END:
        logError(
            ctx,
            pos,
            "The <?teng frag?> directive must contain the frag name "
            "(e.g. <?teng frag example?>; discarding fragment block content"
        );
        break;
    default:
        logError(
            ctx,
            pos,
            "Invalid fragment identifier; discarding fragment block content"
        );
        break;
    }
    // if symbol is invalid then create frag instruction with empty name that
    // is used as marker for close_frag() function to discard frag content
    ctx->open_frames.top().open_frag({}, ctx->program->size(), false);
    generate<OpenFrag_t>(ctx, "", ctx->unexpected_token.pos);
    reset_error(ctx);
}

void close_frag(Context_t *ctx, const Pos_t &pos, bool invalid) {
    // pop as many fragments as needed
    for (; !ctx->open_frames.top().empty();) {
        // close fragment
        auto frag = ctx->open_frames.top().close_frag();

        // create end-frag instruction and take fragment subprogram length
        generate<CloseFrag_t>(ctx, pos);
        auto frag_routine_length = ctx->program->size() - frag.addr - 1;

        // take references to instructions after push_back that invalidates them
        auto &open_frag_instr = open_frag_cast((*ctx->program)[frag.addr]);
        auto &close_frag_instr = ctx->program->back().as<CloseFrag_t>();

        // open frag instr contains offset of close frag instr and vice versa
        open_frag_instr.close_frag_offset = frag_routine_length;
        close_frag_instr.open_frag_offset = -frag_routine_length;

        // if fragment has invalid name discard all code up to open instruction
        if (invalid || frag.name().empty() || open_frag_instr.name.empty())
            ctx->program->erase_from(frag.addr);

        // close frame if is empty
        if (ctx->open_frames.top().empty()) {
            if (ctx->open_frames.size() > 1) {
                ctx->open_frames.close_frame();
                generate<CloseFrame_t>(ctx, pos);
            }
        }

        // if fragment hasn't been created as part of auto fragments then stop
        if (!frag.auto_close) return;
    }
    logWarning(ctx, pos, "Closing frag requested but no one opened!");
}

void close_inv_frag(Context_t *ctx, const Pos_t &pos) {
    close_frag(ctx, pos, true);
    logError(
        ctx,
        pos,
        "Excessive tokens in <?teng endfrag?>; "
        "discarding fragment block content"
    );
    reset_error(ctx);
}

void
close_unclosed_frag(Context_t *ctx, const Pos_t &pos, const Token_t &token) {
    close_frag(ctx, token.pos);
    logWarning(
        ctx,
        pos,
        "The closing directive of this <?teng frag?> directive is missing"
    );
    note_error(ctx, token);
    reset_error(ctx);
}

void debug_frag(Context_t *ctx, const Pos_t &pos, bool warn) {
    generate<DebugFrag_t>(ctx, pos);
    if (warn) {
        logWarning(
            ctx,
            pos,
            "Invalid or excessive tokens in <?teng debug?>; ignoring them"
        );
        reset_error(ctx);
    }
}

void bytecode_frag(Context_t *ctx, const Pos_t &pos, bool warn) {
    generate<BytecodeFrag_t>(ctx, pos);
    if (warn) {
        logWarning(
            ctx,
            pos,
            "Invalid or excessive tokens in <?teng bytecode?>; "
            "ignoring them"
        );
        reset_error(ctx);
    }
}

} // namespace Parser
} // namespace Teng

