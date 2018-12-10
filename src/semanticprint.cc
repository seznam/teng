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
#include "logging.h"
#include "instruction.h"
#include "configuration.h"
#include "parsercontext.h"
#include "semanticprint.h"

#ifdef DEBUG
#include <iostream>
#define DBG(...) __VA_ARGS__
#else /* DEBUG */
#define DBG(...)
#endif /* DEBUG */

namespace Teng {
namespace Parser {
namespace {

/** Returns true if instructions from given address (inclusive) are protected.
 */
bool are_instrs_protected(Context_t *ctx, int64_t from_addr) {
    // addresses of JMP instructions
    if (!ctx->branch_addrs.empty())
        if (!ctx->curr_branch_addrs().empty())
            if (ctx->curr_branch_addrs().top() >= from_addr)
                return true;

    // addresses of JPM instructions of case expression
    if (!ctx->case_option_addrs.empty())
        if (!ctx->case_option_addrs.top().empty())
            if (ctx->case_option_addrs.top().top() >= from_addr)
                return true;

    // start address of if statement
    if (!ctx->if_start_points.empty())
        if (ctx->if_start_points.top().addr >= from_addr)
            return true;

    // optimizations points - they are used only inside expressions
    // if (!ctx->optimization_points.empty())
    //     if (ctx->optimization_points.top().addr >= from_addr)
    //         return true;

    // start address of expression statement
    if (ctx->expr_start_point.addr >= from_addr)
        return true;

    // instructions are not referenced by addresses so can be optimized out
    return false;
}

} // namespace

void generate_print(Context_t *ctx, bool print_escape) {
    // get current program size
    int64_t prgsize = ctx->program->size();

    // underflow protect -> no optimalization can be peformed for now
    if (prgsize < 3)
        return generate<Print_t>(ctx, print_escape, ctx->pos());

    // check whether there is no references to vanishing code
    if (are_instrs_protected(ctx, prgsize - 3))
        return generate<Print_t>(ctx, print_escape, ctx->pos());

    // attempt to optimize consecutive print instrs to one merged
    if ((*ctx->program)[prgsize - 1].opcode() != OPCODE::VAL)
        return generate<Print_t>(ctx, print_escape, ctx->pos());
    if ((*ctx->program)[prgsize - 2].opcode() != OPCODE::PRINT)
        return generate<Print_t>(ctx, print_escape, ctx->pos());
    if ((*ctx->program)[prgsize - 3].opcode() != OPCODE::VAL)
        return generate<Print_t>(ctx, print_escape, ctx->pos());

    // TODO(burlog): can this replace are_instrs_protected and NOOP insertions?

    // check if print can be optimized out
    if ((*ctx->program)[prgsize - 2].as<Print_t>().unoptimizable)
        return generate<Print_t>(ctx, print_escape, ctx->pos());

    DBG(std::cerr << "$$$$ print optimization" << std::endl);

    // optimalize sequence of VAL, PRINT, VAL, PRINT to single VAL, PRINT pair
    auto &first_val = (*ctx->program)[prgsize - 3].as<Val_t>().value;
    auto &second_val = (*ctx->program)[prgsize - 1].as<Val_t>().value;

    // if print escaping is enabled we have to respect print escaping flag
    if (ctx->params->isPrintEscapeEnabled()) {
        auto &print_instr = (*ctx->program)[prgsize - 2].as<Print_t>();
        auto esc = [&] (auto &&v) {return ctx->escaper.escape(v);};
        switch (int(print_escape) - int(print_instr.print_escape)) {
        case 0:  // (true - true) || (false - false)
            first_val.append_str(second_val);
            break;
        case -1: // (false - true)
            second_val.print([&] (const string_view_t &v) {
                first_val.append_str(esc(v));
            });
            print_instr.print_escape = false;
            break;
        case 1:  // (true - false)
            second_val.print([&] (const string_view_t &v2) {
                first_val.print([&] (const string_view_t &v1) {
                    first_val = esc(v1) + v2;
                });
            });
            print_instr.print_escape = false;
            break;
        }

    // or if it is disabled then we can directly join values
    } else first_val.append_str(second_val);

    // delete last VAL instruction (optimized out)
    ctx->program->pop_back();
}

void generate_dict_lookup(Context_t *ctx, const Token_t &token) {
    // find item in dictionary
    if (auto *item = ctx->dict->lookup(token.view()))
        return generate<Val_t>(ctx, *item, token.pos);

    // find item in param/config dictionary
    if (auto *item = ctx->params->lookup(token.view()))
        return generate<Val_t>(ctx, *item, token.pos);

    // use ident as result value
    logWarning(
        ctx,
        token.pos,
        "Dictionary item '" + token.view() + "' was not found"
    );
    generate<Val_t>(ctx, token.str(), token.pos);
}

void generate_raw_print(Context_t *ctx) {
    generate_print(ctx, false);
}

void generate_raw_print(Context_t *ctx, const Token_t &token) {
    generate_val(ctx, token.pos, Value_t(ctx->lex1().unescape(token.view())));
    generate_raw_print(ctx);
}

void generate_inv_print(Context_t *ctx, const Token_t &inv) {
    note_error(ctx, inv);
    reset_error(ctx);
    logWarning(ctx, inv.pos, "Invalid expression; the behaviour is undefined");
    generate_raw_print(ctx);
}

void print_dict_lookup(Context_t *ctx, const Token_t &token) {
    generate_dict_lookup(ctx, token);
    generate_print(ctx);
}

void print_dict_undef(Context_t *ctx, const Token_t &token) {
    generate_val(ctx, token.pos, Value_t());
    logWarning(ctx, token.pos, "Invalid dictionary key in #{...} statement");
    generate_raw_print(ctx);
}

} // namespace Parser
} // namespace Teng

