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

#include "syntax.hh"
#include "program.h"
#include "logging.h"
#include "instruction.h"
#include "parsercontext.h"
#include "configuration.h"
#include "semanticinheritance.h"

#ifdef DEBUG
#include <iostream>
#define DBG(...) __VA_ARGS__
#else /* DEBUG */
#define DBG(...)
#endif /* DEBUG */

namespace Teng {
namespace Parser {
namespace {

using IOverrides_t = OverriddenBlocks_t::Overrides_t::reverse_iterator;

/** Generates recursively all overrides of the block.
 *
 * 003 JMP                 <jump=+1>            (base.html)
 * 004 RETURN
 *
 * 005 JMP                 <jump=+2>            (override1.html)
 * 006 CALL                <addr=3,name=head>
 * 007 RETURN
 *
 * 008 CALL                <addr=6,name=head>   (override2.html)
 */
void generate_overrides(
    Context_t *ctx,
    const std::string &name,
    IOverrides_t ioverride,
    IOverrides_t eoverride
) {
    // update super jump
    auto &instr = (*ctx->program)[ioverride->addr].as<Jmp_t>();
    instr.addr_offset = ctx->program->size() - ioverride->addr - 1;

    // save current super addr, it's used for implementing super()
    ctx->extends_block.super_addr = ioverride->addr;

    // then generate terminal call
    auto inext_override = std::next(ioverride);
    if (inext_override == eoverride) {
        // we are leaving overrides code, so calling super() makes no sense
        ctx->extends_block.super_addr = -1;
        // generates call of the first override
        generate<Call_t>(ctx, name, ioverride->addr, ioverride->pos);
        return;
    }

    // where the override starts
    inext_override->addr = ctx->program->size();
    generate<Jmp_t>(ctx, inext_override->pos);

    // tokenize, parse and generataion of the override code
    ctx->load_source(inext_override->raw_data, &inext_override->pos);

    // at the end of code generation we have to generate RETURN
    // and generate code for next override if any
    ctx->lex1_stack.top().action = [=] {
        generate<Return_t>(ctx, ctx->pos());
        generate_overrides(ctx, name, inext_override, eoverride);
    };
}

} // namespace

void extends_file(Context_t *ctx, const Pos_t &pos, const Options_t &opts) {
    // if there is already open extends block ignore this
    if (++ctx->extends_block.nesting_level > 1) {
        logError(
            ctx,
            pos,
            "There is already open extends block at "
            + ctx->extends_block.pos.str()
            + "; ignoring the extends directive"
        );
        return;
    }

    // ensure that file option exists
    auto iopt = opts.find("file");
    if (iopt == opts.end()) {
        logError(
            ctx,
            pos,
            "Can't extends template; the 'file' option is missing"
        );
        return;
    }

    // ensure that file option is a string
    if (!iopt->value.is_string()) {
        logError(
            ctx,
            pos,
            "Can't extends template; the 'file' value is not string"
        );
        return;
    }

    // ensure that we not go beyond include limit
    if (ctx->lex1_stack.size() >= ctx->params->getMaxIncludeDepth()) {
        logError(ctx, pos, "Can't extends template; include level is too deep");
        return;
    }

    // warn if filename is empty
    if (iopt->value.string().empty()) {
        logWarning(
            ctx,
            pos,
            "Can't extends template; the 'file' value empty string"
        );
    }

    // create new overriding blocks registry
    ctx->extends_block.pos = pos;
    ctx->extends_block.base_file = iopt->value.string().str();
}

void ignore_extends(Context_t *ctx, const Token_t &token, bool empty) {
    // if there is already open extends block ignore this
    if (++ctx->extends_block.nesting_level > 1) {
        logError(
            ctx,
            token.pos,
            "There is already open extends block at "
            + ctx->extends_block.pos.str()
            + "; ignoring the extends directive "
        );
        return;
    }

    // extends block is invalid so warn
    if (empty) {
        logError(
            ctx,
            token.pos,
            "Missing template to extend; ignoring the extends directive"
        );

    } else {
        switch (ctx->unexpected_token) {
        case LEX2_EOF:
        case LEX2::END:
            logError(
                ctx,
                token.pos,
                "Premature end of <?teng extends?> directive"
            );
            break;
        default:
            logError(
                ctx,
                token.pos,
                "Invalid or excessive tokens in <?teng extends?>"
            );
            break;
        }
        reset_error(ctx);
    }
}

void close_extends(Context_t *ctx, const Pos_t *inv_pos) {
    if (inv_pos)
        logWarning(ctx, *inv_pos, "Ignoring invalid or excessive tokens");

    // be sure that nesting level counter don't underflow
    if (ctx->extends_block.nesting_level <= 0)
        throw std::runtime_error(__PRETTY_FUNCTION__);

    // we are leaving extends block
    // if it is not top-level then skip reading the base template
    if (--ctx->extends_block.nesting_level)
        return;

    // ignore empty string
    if (ctx->extends_block.base_file.empty())
        return;

    // compile file (append compiled file at the end of current program)
    auto &extends_block = ctx->extends_block;
    ctx->load_file(extends_block.base_file, extends_block.pos);
}

void note_override_block(Context_t *ctx, const Token_t &ident) {
    // this switches level 2 lexer to override_block start condition
    ctx->lex2().switch_to_override_block();

    // open override block and note its address
    auto &extends_block = ctx->extends_block;
    extends_block.open_override(ctx, ident.str(), -1);
}

void reg_override_block(Context_t *ctx, const Token_t &end_block) {
    // this switches level 2 lexer to initial start condition
    ctx->lex2().switch_to_initial();

    // skip unopened blocks
    if (!ctx->extends_block.is_override_block_open())
        return;

    // close override block and remember its source code range
    auto &extends_block = ctx->extends_block;
    auto &name = extends_block.override_block().name;
    auto block = extends_block.close_override(end_block.token_view.begin());
    ctx->overridden_blocks.reg_block(std::move(name), std::move(block));
}

void note_define_block(Context_t *ctx, const Token_t &ident) {
    // open override for base implementation and note its address
    auto &extends_block = ctx->extends_block;
    extends_block.open_override(ctx, ident.str(), ctx->program->size());

    // generate jump instruction to overrides
    generate<Jmp_t>(ctx, ident.pos);
}

void note_inv_define_block(Context_t *ctx, const Token_t &define_block) {
    logWarning(
        ctx,
        define_block.pos,
        "Ignoring define block with invalid block id"
    );
}

void reg_define_block(Context_t *ctx, const Token_t &end_block) {
    // skip unopened blocks
    if (!ctx->extends_block.is_override_block_open())
        return;

    // generate subroutine return instruction
    generate<Return_t>(ctx, end_block.pos);

    // the code for the base implementation has been generated
    auto &blocks = ctx->overridden_blocks;
    auto &extends_block = ctx->extends_block;
    auto &name = extends_block.override_block().name;
    auto block = extends_block.close_override(end_block.token_view.begin());
    auto &entry = blocks.reg_block(std::move(name), std::move(block));

    // recursively generate all its overrides
    auto &overrides = entry.second;
    generate_overrides(ctx, entry.first, overrides.rbegin(), overrides.rend());
}

void call_super_block(Context_t *ctx, const Token_t &super_block) {
    // calling super out of overriding block makes no sense
    if (ctx->extends_block.super_addr < 0) {
        logWarning(ctx, super_block.pos, "There is no open overriding blocks");
        return;
    }

    // generate instructions calling super implementation
    auto super_addr = ctx->extends_block.super_addr;
    generate<Call_t>(ctx, "super", super_addr, super_block.pos);
}

void ignore_free_override(Context_t *ctx, const Token_t &token) {
    logError(
        ctx,
        token.pos,
        "The misplaced " + token.token_name() + " token, it has to be placed "
        "in <?teng extends?> block"
    );
}

void ignore_unclosed_extends(Context_t *ctx, const Token_t &token) {
    logError(ctx, token.pos, "The unclosed <?teng extends?>; ignoring it");
}

void note_inv_override_block(Context_t *ctx, const Token_t &override_block) {
    logWarning(
        ctx,
        override_block.pos,
        "Ignoring override block with invalid block id"
    );
}

void unclosed_define_block(Context_t *ctx, const Token_t &token) {
    logError(ctx, token.pos, "The <?teng define block?> is not closed");
    reg_define_block(ctx, token);
}

} // namespace Parser
} // namespace Teng

