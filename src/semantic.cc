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
#include <algorithm>

#include "syntax.hh"
#include "program.h"
#include "formatter.h"
#include "parserfrag.h"
#include "dictionary.h"
#include "contenttype.h"
#include "configuration.h"
#include "semantic.h"

#ifdef DEBUG
#define DBG(...) __VA_ARGS__
#else /* DEBUG */
#define DBG(...)
#endif /* DEBUG */

namespace Teng {
namespace Parser {
namespace {

/** Generates set var instruction.
 */
void set_var_impl(Context_t *ctx, const Variable_t &var) {
    switch (var.id) {
    case LEX2::BUILTIN_FIRST:
    case LEX2::BUILTIN_INNER:
    case LEX2::BUILTIN_LAST:
    case LEX2::BUILTIN_INDEX:
    case LEX2::BUILTIN_COUNT:
    case LEX2::BUILTIN_PARENT:
    case LEX2::BUILTIN_THIS:
        logError(
            ctx,
            var.pos,
            "Builtin variable '" + var.str() + "' can't be set"
        );
        ctx->program->erase_from(ctx->expr_start_point.addr);
        break;

    case LEX2::BUILTIN_ERROR:
        logError(
            ctx,
            var.pos,
            "Builtin fragment '" + var.str() + "' can't be set"
        );
        ctx->program->erase_from(ctx->expr_start_point.addr);
        break;

    case LEX2::VAR:   // $ident
    case LEX2::IDENT: // ident
        if (var.ident.name().view()[0] == '_') {
            if (var.ident.name().view().size() > 1) {
                if (var.ident.name().view()[1] != '_') {
                    logWarning(
                        ctx,
                        var.pos,
                        "The variable names starting with an underscore are "
                        "reserved, and might cause undefined behaviour in "
                        "future: var=" + var.symbol_view
                    );
                }
            }
        }
        // pass through
    case LEX2::LT_DIGRAPH:
    case LEX2::LE_DIGRAPH:
    case LEX2::GT_DIGRAPH:
    case LEX2::GE_DIGRAPH:
    case LEX2::EQ_DIGRAPH:
    case LEX2::NE_DIGRAPH:
    case LEX2::AND_TRIGRAPH:
    case LEX2::OR_DIGRAPH:
    case LEX2::DEFINED:
    case LEX2::REPR:
    case LEX2::ISEMPTY:
    case LEX2::EXISTS:
    case LEX2::TYPE:
    case LEX2::COUNT:
    case LEX2::CASE:
        generate<Set_t>(ctx, var);
        break;

    default:
        logError(
            ctx,
            var.pos,
            "Invalid variable identifier: ident=" + var.symbol_view
            + ", token=" + l2_token_name(var.id)
        );
        break;
    }
}

/** Generates var instruction.
 */
void generate_var_impl(Context_t *ctx, const Variable_t &var) {
    // generate var instruction
    switch (var.id) {
    case LEX2::BUILTIN_FIRST:
        generate<PushFragFirst_t>(ctx, var);
        break;
    case LEX2::BUILTIN_INNER:
        generate<PushFragInner_t>(ctx, var);
        break;
    case LEX2::BUILTIN_LAST:
        generate<PushFragLast_t>(ctx, var);
        break;
    case LEX2::BUILTIN_INDEX:
        generate<PushFragIndex_t>(ctx, var);
        break;
    case LEX2::BUILTIN_COUNT:
        generate<PushFragCount_t>(ctx, var);
        break;
    case LEX2::BUILTIN_THIS:
        generate<PushFrag_t>(ctx, var);
        break;
    case LEX2::BUILTIN_PARENT:
        if (var.offset.frag >= ctx->open_frames.top().size()) {
            logWarning(
                ctx,
                var.pos,
                "The builtin _parent variable has crossed root boundary; "
                "converting it to _this"
            );
            generate<PushFrag_t>(ctx, var);
        } else generate<PushFrag_t>(ctx, var, var.offset.frag + 1);
        break;

    case LEX2::BUILTIN_ERROR:
        ctx->params->isErrorFragmentEnabled()
            ? generate<PushErrorFrag_t>(ctx, false, var.pos)
            : generate<Var_t>(ctx, var, true);
        break;

    case LEX2::VAR:   // $ident
    case LEX2::IDENT: // ident
        if (var.ident.name().view()[0] == '_') {
            if (var.ident.name().view().size() > 1) {
                if (var.ident.name().view()[1] != '_') {
                    logWarning(
                        ctx,
                        var.pos,
                        "The variable names starting with an underscore are "
                        "reserved, and might cause undefined behaviour in "
                        "future: var=" + var.symbol_view
                    );
                }
            }
        }
        // pass through

    case LEX2::LT_DIGRAPH:
    case LEX2::LE_DIGRAPH:
    case LEX2::GT_DIGRAPH:
    case LEX2::GE_DIGRAPH:
    case LEX2::EQ_DIGRAPH:
    case LEX2::NE_DIGRAPH:
    case LEX2::AND_TRIGRAPH:
    case LEX2::OR_DIGRAPH:
    case LEX2::DEFINED:
    case LEX2::REPR:
    case LEX2::ISEMPTY:
    case LEX2::EXISTS:
    case LEX2::TYPE:
    case LEX2::COUNT:
    case LEX2::CASE:
        generate<Var_t>(ctx, var, true);
        break;

    default:
        logError(
            ctx,
            var.pos,
            "Invalid variable identifier: ident=" + var.symbol_view
            + ", token=" + l2_token_name(var.id)
        );
        break;
    }
}

/** Converts relative variable to absolute.
 *
 * Assumption: var_sym offsets are valid
 */
Identifier_t make_absolute_ident(Context_t *ctx, const Variable_t &var_sym) {
    Identifier_t abs_ident(/*relative*/false);

    // get variable frame
    auto &frame = *(ctx->open_frames.end() - var_sym.offset.frame - 1);

    // get variable open fragments prefix
    auto path_size = frame.size() - var_sym.offset.frag;
    auto ident_suffix_size = var_sym.ident.size() - 1; // omit variable name
    auto root_prefix_size = path_size - ident_suffix_size;

    // copy path from root to the first segment of the variable identifier
    auto ifrag = frame.begin();
    auto efrag = frame.begin() + root_prefix_size;
    for (; ifrag != efrag; ++ifrag)
        abs_ident.push_back(ifrag->token);

    // then copy all segments in variable identifier
    for (auto &segment: var_sym.ident)
        abs_ident.push_back(segment);

    return abs_ident;
}

/** Local variable has scalar offsets.
 */
Variable_t::Offset_t resolve_local_var(Context_t *) {
    return {0, 0};
}

/** Attemps resolve relative variable in any open frames in reverse order. It
 * does not do _ANY_ check that variable is relative. The examples explains
 * resolution better than long explanations.
 *
 * frames: .a.b.c, .a.d, .a.b
 *
 * then ident b.x refers to .a.b.x in current frame
 * then ident a.b.x refers to .a.b.x in current frame
 * then ident a.x refers to .a.x in current frame
 *
 * then ident d.x refers to .a.d.x in -1 frame
 * then ident a.d.x refers to .a.d.x in -1 frame
 *
 * then ident c.x refers to .a.b.c.x in -2 frame
 * then ident a.b.c.x refers to .a.b.c.x in -2 frame
 */
Variable_t::Offset_t
resolve_relative_var(Context_t *ctx, const Variable_t &var_sym) {
    // initialied to invalid offsets
    Variable_t::Offset_t offset;

    // try resolve relative variable path in any open frames
    auto irframe = ctx->open_frames.rbegin();
    auto erframe = ctx->open_frames.rend();
    for (; irframe != erframe; ++irframe) {

        // does open fragments in current frame contain our ident
        auto irel_start_at = irframe->resolve_relative(var_sym.ident);
        if (irel_start_at != irframe->end()) {

            // the size of path from root to the first var identifier
            auto root_prefix_size = irel_start_at - irframe->begin();

            // the size of path from ident (omit variable name)
            auto ident_suffix_size = var_sym.ident.size() - 1;

            // the whole path is size of root prefix plus ident size
            auto path_size = root_prefix_size + ident_suffix_size;

            // calc offsets
            offset.frame = irframe - ctx->open_frames.rbegin();
            offset.frag = irframe->size() - path_size;
            break;
        }
    }
    return offset;
}

/** Absolute vars can refer to variables in any open frame and open fragment
 * but not to closed one.
 */
Variable_t::Offset_t
resolve_abs_var(Context_t *ctx, const Variable_t &var_sym) {
    // initialied to invalid offsets
    Variable_t::Offset_t offset;

    // find frame where open frags list matches variable path (in reverse order)
    auto irframe = ctx->open_frames.rbegin();
    auto erframe = ctx->open_frames.rend();
    for (; irframe != erframe; ++irframe) {

        // the identifier must not be zero here (-1 is for variable name)
        std::size_t ident_size = var_sym.ident.size() - 1;
        for (std::size_t i = 0, frag_count = irframe->size();; ++i) {
            // if we match whole identifier then we resolved the var
            if (i >= ident_size) {
                offset.frame = irframe - ctx->open_frames.rbegin();
                offset.frag = irframe->size() - i;
                return offset;
            }

            // if identifier is longer than count of open frags then variable
            // can't be resolved in current frame, so let's try another one
            if (i >= frag_count)
                break;

            // if current identifier segment does not match open frame name
            // then variable can't be resolved in current frame, so let's try
            // another one
            if (var_sym.ident[i].view() != (*irframe)[i].name())
                break;
        }
    }
    return offset;
}

/** Generates runtime variable path instructions for desired variable.
 *
 * Assumption: var_sym.ident.size() > 1.
 */
template <bool gen_repr>
void
generate_auto_rtvar_path(
    Context_t *ctx,
    const Variable_t &var_sym,
    const char *path_end
) {
    // it is used to generate instruction for one identifier segment
    auto path_start = var_sym.view().begin();

    // generate runtime variable from regular variable
    for (std::size_t i = 0; i < var_sym.ident.size(); ++i) {
        auto &segment = var_sym.ident[i];
        string_view_t path = {path_start, path_end};
        switch (segment.token_id) {
        case LEX2::BUILTIN_FIRST:
            generate<PushValFirst_t>(ctx, path.str(), var_sym.pos);
            break;
        case LEX2::BUILTIN_INNER:
            generate<PushValInner_t>(ctx, path.str(), var_sym.pos);
            break;
        case LEX2::BUILTIN_LAST:
            generate<PushValLast_t>(ctx, path.str(), var_sym.pos);
            break;
        case LEX2::BUILTIN_INDEX:
            generate<PushValIndex_t>(ctx, path.str(), var_sym.pos);
            break;
        case LEX2::BUILTIN_COUNT:
            generate<PushValCount_t>(ctx, path.str(), var_sym.pos);
            break;

        case LEX2::BUILTIN_PARENT:
            if (var_sym.ident.size() == 1) {
                logWarning(
                    ctx,
                    var_sym.pos,
                    "The builtin _parent variable has crossed root boundary; "
                    "converting it to _this"
                );
            } else ctx->program->erase_from(ctx->program->size() - 1);
            break;

        case LEX2::BUILTIN_THIS:
            break;

        case LEX2::BUILTIN_ERROR:
            if (ctx->params->isErrorFragmentEnabled()) {
                generate<PushErrorFrag_t>(ctx, true, var_sym.pos);
                break;
            }
            // pass through

        default:
            generate<PushAttr_t>(ctx, segment.str(), path.str(), var_sym.pos);
            if (gen_repr && (i == (var_sym.ident.size() - 1)))
                generate<Repr_t>(ctx, var_sym.pos);
            break;
        }
        path_end = segment.view().end();
    }
}

/** Generates runtime variable instructions for desired variable.
 *
 * Assumption: var_sym.ident.size() > 1.
 */
template <bool gen_repr>
void generate_auto_rtvar(Context_t *ctx, const Variable_t &var_sym) {
    // relative variables
    if (var_sym.ident.is_relative()) {
        generate<PushThisFrag_t>(ctx, var_sym.pos);
        auto path_end = var_sym.view().begin();
        return generate_auto_rtvar_path<gen_repr>(ctx, var_sym, path_end);
    }

    // absolute variables - no open fragments
    if (ctx->open_frames.top().empty()) {
        generate<PushRootFrag_t>(ctx, uint16_t(0), var_sym.pos);
        auto path_end = var_sym.view().begin();
        return generate_auto_rtvar_path<gen_repr>(ctx, var_sym, path_end);
    }

    // absolute variables - there is at least one open fragment
    for (uint16_t i = 0;; ++i) {
        // match common prefix (omit variable name)
        if (i < ctx->open_frames.top().size())
            if (std::size_t(i + 1) < var_sym.ident.size())
                if (ctx->open_frames.top()[i].name() == var_sym.ident[i].view())
                    continue;
        Identifier_t ident;
        for (auto j = i; j < var_sym.ident.size(); ++j)
            ident.push_back(var_sym.ident[j]);
        Variable_t rel_var(var_sym, std::move(ident));
        generate<PushThisFrag_t>(ctx, var_sym.pos);
        auto path_end = i
            ? var_sym.ident[i - 1].view().end()
            : var_sym.ident[i].view().begin();
        return generate_auto_rtvar_path<gen_repr>(ctx, rel_var, path_end);
    }
}

/** Generates variable lookup.
 */
template <bool gen_repr>
void generate_var_templ(Context_t *ctx, Variable_t var) {
    // instruction for local variables can be generated immediately
    if (var.ident.is_local()) {
        var.offset = resolve_local_var(ctx);
        return generate_var_impl(ctx, var);
    }

    // absolute variables have to be resolved prior to instruction generation
    if (var.ident.is_absolute()) {
        if ((var.offset = resolve_abs_var(ctx, var)))
            return generate_var_impl(ctx, var);
        // resolution failed so convert scalar variable to runtime
        return generate_auto_rtvar<gen_repr>(ctx, var);
    }

    // relative variables have to be resolved prior to instruction generation
    if ((var.offset = resolve_relative_var(ctx, var))) {
        var.ident = make_absolute_ident(ctx, var);
        return generate_var_impl(ctx, var);
    }
    // resolution failed so convert scalar variable to runtime
    return generate_auto_rtvar<gen_repr>(ctx, var);
}

/** Generates expression open frag instruction.
 */
void open_frag(Context_t *ctx, const Token_t &token, const Pos_t &pos) {
    (token == LEX2::BUILTIN_ERROR) && ctx->params->isErrorFragmentEnabled()
        ? generate<OpenErrorFrag_t>(ctx, pos)
        : generate<OpenFrag_t>(ctx, token.view().str(), pos);
}

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

void generate_var(Context_t *ctx, Variable_t var) {
    generate_var_templ<true>(ctx, std::move(var));
}

void prepare_root_variable(Context_t *ctx, const Token_t &token) {
    ctx->var_sym = Variable_t(token, Identifier_t(false));
}

void prepare_this_variable(Context_t *ctx, const Token_t &token) {
    Identifier_t result(false);
    for (auto &frag: ctx->open_frames.top())
        result.push_back(frag.token);
    ctx->var_sym = Variable_t(token, std::move(result));
}

void prepare_parent_variable(Context_t *ctx, const Token_t &token) {
    Identifier_t result(false);
    if (!ctx->open_frames.top().empty()) {
        for (auto i = 0lu; i < ctx->open_frames.top().size() - 1; ++i)
            result.push_back(ctx->open_frames.top()[i].token);
    } else logWarning(ctx, token.pos, "The _parent violates the root boundary");
    ctx->var_sym = Variable_t(token, std::move(result));
}

void generate_print(Context_t *ctx, bool print_escape) {
    // get current program size
    auto prgsize = ctx->program->size();

    // underflow protect -> no optimalization can be peformed for now
    if (prgsize < 3)
        return generate<Print_t>(ctx, print_escape, ctx->pos());

    // attempt to optimize consecutive print instrs to one merged
    if ((*ctx->program)[prgsize - 1].opcode() != OPCODE::VAL)
        return generate<Print_t>(ctx, print_escape, ctx->pos());
    if ((*ctx->program)[prgsize - 2].opcode() != OPCODE::PRINT)
        return generate<Print_t>(ctx, print_escape, ctx->pos());
    if ((*ctx->program)[prgsize - 3].opcode() != OPCODE::VAL)
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

    // or if it is disabled we can directly join values
    } else first_val.append_str(second_val);

    // delete last VAL instruction (optimized out)
    ctx->program->pop_back();
}

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
        auto &open_frag_instr = (*ctx->program)[frag.addr].as<OpenFrag_t>();
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

void generate_val(Context_t *ctx, const Pos_t &pos, Value_t value) {
    generate<Val_t>(ctx, std::move(value), pos);
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

void generate_tern_op(Context_t *ctx, const Token_t &token) {
    ctx->branch_addrs.top().push(ctx->program->size());
    generate<JmpIfNot_t>(ctx, token.pos);
    expr_diag(ctx, diag_code::tern_true_branch, false);
}

void finalize_tern_op_true_branch(Context_t *ctx, const Token_t &token) {
    // calculate jump from begin of tern op to false branch
    auto cond_addr = ctx->branch_addrs.top().pop();
    auto false_branch_offset = ctx->program->size() - cond_addr;

    // store address of jump from true branch
    ctx->branch_addrs.top().push(ctx->program->size());
    generate<Jmp_t>(ctx, token.pos);

    // fix conditional jump offset (relative addr)
    auto &instr = (*ctx->program)[cond_addr].as<JmpIfNot_t>();
    instr.addr_offset = false_branch_offset;

    // diagnostic code
    expr_diag(ctx, diag_code::tern_false_branch);
}

void finalize_tern_op_false_branch(Context_t *ctx) {
    auto true_branch_jump_addr = ctx->branch_addrs.top().pop();
    auto tern_op_end_offset = ctx->program->size() - true_branch_jump_addr - 1;
    auto &instr = (*ctx->program)[true_branch_jump_addr].as<Jmp_t>();
    instr.addr_offset = tern_op_end_offset;
    ctx->expr_diag.pop();

    // breaks invalid print optimization
    generate<Noop_t>(ctx);
}

void generate_query(Context_t *ctx, const Variable_t &var, bool warn) {
    generate<LogSuppress_t>(ctx, var.pos);
    note_optimization_point(ctx, true);
    generate_var_templ<false>(ctx, var);

    // warn if query is name($some.var)
    if (warn) {
        logWarning(
            ctx,
            var.pos,
            "In query expression the identifier shouldn't be denoted by $ sign"
        );
    }
}

void include_file(Context_t *ctx, const Pos_t &pos, const Options_t &opts) {
    // ensure that file option exists
    auto iopt = opts.find("file");
    if (iopt == opts.end()) {
        logError(ctx, pos, "Can't include file; the 'file' option is missing");
        return;
    }

    // ensure that we not go beyond include limit
    if (ctx->lex1_stack.size() >= ctx->params->getMaxIncludeDepth()) {
        logError(ctx, pos, "Can't include file; include level is too deep");
        return;
    }

    // compile file (append compiled file at the end of current program)
    ctx->load_file(iopt->value.string(), pos);
}

void ignore_include(Context_t *ctx, const Token_t &token, bool empty) {
    if (empty) {
        logError(
            ctx,
            token.pos,
            "Missing filename to include; ignoring the include directive"
        );
    } else {
        switch (ctx->unexpected_token) {
        case LEX2_EOF:
        case LEX2::END:
            logError(
                ctx,
                token.pos,
                "Premature end of <?teng include?> directive; "
                "ignoring the include directive"
            );
            break;
        default:
            logError(
                ctx,
                token.pos,
                "Invalid or excessive tokens in <?teng include?>; "
                "ignoring the include directive"
            );
            break;
        }
        reset_error(ctx);
    }
}

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
        auto addr = ctx->branch_addrs.top().pop();
        auto &instr = (*ctx->program)[addr].as<Or_t>();
        instr.addr_offset = ctx->program->size() - addr - 1;
    }
    ctx->branch_addrs.top().push(ctx->program->size());
    generate<JmpIfNot_t>(ctx, token.pos);
}

uint32_t
generate_case_next(Context_t *ctx, Literal_t &literal, uint32_t alts) {
    ctx->branch_addrs.top().push(ctx->program->size());
    generate<Or_t>(ctx, literal.pos);
    generate_case_cmp(ctx, literal);
    return alts + 1;
}

void finalize_case_branch(Context_t *ctx, const Token_t &token) {
    auto branch_case_addr = ctx->branch_addrs.top().pop();
    auto &instr = (*ctx->program)[branch_case_addr].as<JmpIfNot_t>();
    instr.addr_offset = ctx->program->size() - branch_case_addr;
    ctx->branch_addrs.top().push(ctx->program->size());
    generate<Jmp_t>(ctx, token.pos);
}

NAryExpr_t finalize_case(Context_t *ctx, const Token_t &token, uint32_t arity) {
    // update addr offsets of jmp instruction at the end of all branches
    for (auto tmp_arity = arity; --tmp_arity;) {
        auto branch_end_addr = ctx->branch_addrs.top().pop();
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

void expr_diag(Context_t *ctx, diag_code_type new_diag_code, bool pop) {
    if (pop) ctx->expr_diag.pop();
    ctx->expr_diag.push({new_diag_code, ctx->pos()});
}

void
open_format(Context_t *ctx, const Pos_t &pos, const Options_t &opts) {
    auto resolve_mode_id = [&] (auto iopt) {
        // ensure that space option exists
        if (iopt == opts.end()) {
            logError(
                ctx,
                pos,
                "Formatting block has no effect; option 'space' is missing"
            );
            return Formatter_t::MODE_COPY_PREV;
        }

        // ensure that space is string
        if (!iopt->value.is_string_like()) {
            logError(
                ctx,
                pos,
                "Formatting block has no effect; option 'space' is not string"
            );
            return Formatter_t::MODE_COPY_PREV;
        }

        // ensure that space has value
        if (iopt->value.string().empty()) {
            logError(
                ctx,
                pos,
                "Formatting block has no effect; option 'space' is empty"
            );
            return Formatter_t::MODE_COPY_PREV;
        }

        // check space option
        Formatter_t::Mode_t mode_id = resolveFormat(iopt->value.string());
        if (mode_id == Formatter_t::MODE_INVALID) {
            logError(
                ctx,
                pos,
                "Unsupported value '" + iopt->value.string() + "' "
                "of 'space' formatting option"
            );
            return Formatter_t::MODE_COPY_PREV;
        }
        return mode_id;
    };

    // generate format instruction
    auto iopt = opts.find("space");
    generate<OpenFormat_t>(ctx, resolve_mode_id(iopt), pos);
}

void open_inv_format(Context_t *ctx, const Pos_t &pos) {
    switch (ctx->unexpected_token) {
    case LEX2::END:
        logError(
            ctx,
            pos,
            "The <?teng format?> directive must contain at least one option in "
            "name=value format (e.g. space='nospace'); ignoring this directive"
        );
        break;
    default:
        logError(
            ctx,
            pos,
            "Invalid excessive tokens in <?teng format?> directive found; "
            "ignoring this directive"
        );
        break;
    }
    reset_error(ctx);
    generate<OpenFormat_t>(ctx, Formatter_t::MODE_COPY_PREV, pos);
}

void close_format(Context_t *ctx, const Pos_t &pos) {
    generate<CloseFormat_t>(ctx, pos);
}

void close_inv_format(Context_t *ctx, const Pos_t &pos) {
    close_format(ctx, pos);
    logWarning(
        ctx,
        pos,
        "Ignoring invalid excessive tokens in <?teng endformat?> directive"
    );
    reset_error(ctx);
}

void
close_unclosed_format(Context_t *ctx, const Pos_t &pos, const Token_t &token) {
    close_format(ctx, token.pos);
    logWarning(
        ctx,
        pos,
        "The closing directive of this <?teng format?> directive is missing"
    );
    note_error(ctx, token);
    reset_error(ctx);
}

void open_ctype(Context_t *ctx, const Pos_t &pos, const Literal_t &type) {
    // push new ctype instruction
    if (auto *desc = ContentType_t::find(type.value.string())) {
        generate<OpenCType_t>(ctx, desc, pos);
        ctx->escaper.push(desc->contentType.get());
        return;
    }

    // push invalid ctype
    generate<OpenCType_t>(ctx, nullptr, pos);
    ctx->escaper.push(nullptr);

    // log invalid conent type name
    logError(
        ctx,
        pos,
        "Invalid content type '" + type.value.string() + "'"
        "; using top instead"
    );
}

void open_inv_ctype(Context_t *ctx, const Pos_t &pos) {
    switch (ctx->unexpected_token) {
    case LEX2::END:
        logError(
            ctx,
            pos,
            "The <?teng ctype?> directive must contain content type name "
            "(e.g. <?teng ctype 'text/html'?>; using top content type instead"
        );
        break;
    default:
        logError(
            ctx,
            pos,
            "Invalid or excessive tokens in <?teng ctype?> directive"
            "; using top content type instead"
        );
        break;
    }
    generate<OpenCType_t>(ctx, nullptr, pos);
    ctx->escaper.push(nullptr);
    reset_error(ctx);
}

void close_ctype(Context_t *ctx, const Pos_t &pos) {
    ctx->escaper.pop();
    generate<CloseCType_t>(ctx, pos);
}

void close_inv_ctype(Context_t *ctx, const Pos_t &pos) {
    close_ctype(ctx, pos);
    logWarning(
        ctx,
        pos,
        "Ignoring invalid excessive tokens in <?teng endctype?> directive"
    );
    reset_error(ctx);
}

void
close_unclosed_ctype(Context_t *ctx, const Pos_t &pos, const Token_t &token) {
    close_ctype(ctx, token.pos);
    logWarning(
        ctx,
        pos,
        "The closing directive of this <?teng ctype?> directive is missing"
    );
    note_error(ctx, token);
    reset_error(ctx);
}

void prepare_if(Context_t *ctx, const Pos_t &pos) {
    ctx->branch_addrs.push();
    ctx->if_stmnt_start_point = {
        pos,
        static_cast<int64_t>(ctx->program->size()),
        true
    };
}

void generate_if(Context_t *ctx, const Token_t &token, bool valid_expr) {
    // generate if condition
    ctx->branch_addrs.top().push(ctx->program->size());
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

void finalize_if_branch(Context_t *ctx, int32_t shift) {
    // TODO(burlog): optimalize if

    // calculate real jump address for last elif/else branch
    auto branch_addr = ctx->branch_addrs.top().pop();
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

void finalize_if(Context_t *ctx) {
    finalize_if_branch(ctx, 1);
    while (!ctx->branch_addrs.top().empty()) {
        auto branch_addr = ctx->branch_addrs.top().pop();
        auto &instr = (*ctx->program)[branch_addr].as<Jmp_t>();
        instr.addr_offset = ctx->program->size() - branch_addr - 1;
    }

    // breaks invalid print optimization
    generate<Noop_t>(ctx);
}

void finalize_inv_if(Context_t *ctx, const Pos_t &pos) {
    finalize_if(ctx);
    logWarning(
        ctx,
        pos,
        "Ignoring invalid excessive tokens in <?teng endif?> directive"
    );
    reset_error(ctx);
}

void generate_else(Context_t *ctx, const Token_t &token) {
    finalize_if_branch(ctx, 0);
    ctx->branch_addrs.top().push(ctx->program->size());
    generate<Jmp_t>(ctx, token.pos);
}

void generate_inv_else(Context_t *ctx, const Token_t &token) {
    generate_else(ctx, token);
    logWarning(
        ctx,
        token.pos,
        "Ignoring invalid excessive tokens in <?teng else?> directive"
    );
    reset_error(ctx);
}

void generate_elif(Context_t *ctx, const Token_t &token) {
    finalize_if_branch(ctx, 0);
    ctx->branch_addrs.top().push(ctx->program->size());
    generate<Jmp_t>(ctx, token.pos);
}

void finalize_if_stmnt(Context_t *ctx) {
    ctx->branch_addrs.pop();
}

void finalize_inv_if_stmnt(Context_t *ctx, const Token_t &token) {
    auto &pos = ctx->if_stmnt_start_point.pos;
    switch (token) {
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
    ctx->branch_addrs.pop();
    ctx->program->erase_from(ctx->if_stmnt_start_point.addr);
    note_error(ctx, token);
    reset_error(ctx);
}

void discard_if(Context_t *ctx) {
    logError(
        ctx,
        ctx->if_stmnt_start_point.pos,
        "Disordered elif/else branches in <?teng if?> statement; "
        "discarding whole if statement"
    );
    ctx->program->erase_from(ctx->if_stmnt_start_point.addr);
}

void set_var(Context_t *ctx, Variable_t var) {
    // ensure valid variable name
    if (var.ident.empty()) {
        logError(ctx, var.pos, "Invalid variable identifier; it won't be set");
        return;
    }

    // instruction for local variables can be generated immediately
    if (var.ident.is_local()) {
        var.offset = resolve_local_var(ctx);
        return set_var_impl(ctx, var);
    }

    // absolute variables have to be resolved prior to instruction generation
    if (var.ident.is_absolute()) {
        if ((var.offset = resolve_abs_var(ctx, var)))
            return set_var_impl(ctx, var);

        // resolution failed so report error
        ctx->program->erase_from(ctx->expr_start_point.addr);
        return logError(
            ctx,
            var.pos,
            "Variable '" + var.ident.str()
            + "' does not match any open fragment in any open frame; "
            "nothing set"
        );
    }

    // relative variables have to be resolved prior to instruction generation
    if ((var.offset = resolve_relative_var(ctx, var))) {
        var.ident = make_absolute_ident(ctx, var);
        return set_var_impl(ctx, var);
    }

    // resolution failed so report error
    ctx->program->erase_from(ctx->expr_start_point.addr);
    return logError(
        ctx,
        var.pos,
        "Variable '" + var.ident.str()
        + "' does not match any open fragment in any open frame"
    );
}

void ignore_inv_set(Context_t *ctx, const Pos_t &pos) {
    logWarning(
        ctx,
        pos,
        "Invalid variable name in the <?teng set?> directive; "
        "ignoring the set directive"
    );
    reset_error(ctx);
}

void
generate_rtvar_index(Context_t *ctx, const Token_t &lp, const Token_t &rp) {
    auto &rtvar_string = ctx->rtvar_strings.back();
    generate<PushAttrAt_t>(ctx, rtvar_string.str(), lp.pos);

    // remove optimization point of index expression because it breaks
    // "unarity" of rtvar expression and expression optimization routine pops
    // too less optimization points hence it starts from wrong address
    auto optimizable = ctx->optimization_points.top().optimizable;
    ctx->optimization_points.pop();
    ctx->optimization_points.top().optimizable &= optimizable;

    // this code remains valid until the runtime variables
    // will not be broken to more strings
    rtvar_string = {ctx->rtvar_strings.back().begin(), rp.view().end()};
}

counted_ptr<Regex_t> generate_regex(Context_t *ctx, const Token_t &regex) {
    regex_flags_t flags;
    auto i = regex.view().size() - 1;
    for (; regex.view()[i] != '/'; --i) {
        switch (regex.view()[i]) {
        case 'i': flags->ignore_case = true; break;
        case 'g': flags->global = true; break;
        case 'm': flags->multiline = true; break;
        case 'e': flags->extended = true; break;
        case 'X': flags->extra = true; break;
        case 'U': flags->ungreedy = true; break;
        case 'A': flags->anchored = true; break;
        case 'D': flags->dollar_endonly = true; break;
        default:
            logWarning(
                ctx,
                regex.pos,
                "Ignoring unknown regex flag '"
                + std::string(1, regex.view()[i])
                + "'"
            );
        }
    }
    string_view_t pattern = {regex.view().data() + 1, regex.view().data() + i};
    return make_counted<Regex_t>(pattern, flags);
}

void
generate_match(Context_t *ctx, const Token_t &token, const Token_t &regex) {
    // parse regex (split pattern and flags)
    auto regex_value = generate_regex(ctx, regex);

    // generate instruction for parsed regex
    generate<MatchRegex_t>(ctx, std::move(regex_value), regex.pos);
    if (token == LEX2::STR_NE)
        generate<Not_t>(ctx, token.pos);
}

/** Generates instructions implementing the function call.
 */
uint32_t generate_func(Context_t *ctx, const Token_t &name, uint32_t nargs) {
    if (!ctx->params->isPrintEscapeEnabled()) {
        // be optimal for unescape($variable)
        // if last instr. is VAR and should be escaped
        // unescaping a single variable -- change escaping status of that var
        if ((nargs == 1) && (name.view() == "unescape")) {
            if (ctx->program->back().opcode() == OPCODE::VAR) {
                auto &instr = ctx->program->back().as<Var_t>();
                if (instr.escape) {
                    instr.escape = false;
                    return nargs;
                }
            }
        }
    }

    // also include diagnostic message if there any
    ctx->expr_diag.pop();
    ctx->expr_diag.unwind(ctx, ctx->unexpected_token);

    // regular function
    bool is_udf = name.token_id == LEX2::UDF_IDENT;
    generate<Func_t>(ctx, name.str(), nargs, name.pos, is_udf);
    return nargs;
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

void ignore_unknown_directive(Context_t *ctx, const Token_t &token) {
    logError(ctx, token.pos, "Unknown Teng directive: " + token.view() + "?>");
    reset_error(ctx);
}

void ignore_excessive_options(Context_t *ctx, const Pos_t &pos) {
    logWarning(
        ctx,
        pos,
        "This directive doesn't accept any options; ignoring them"
    );
}

void new_option(Context_t *ctx, const Token_t &name, Literal_t &&literal) {
    ctx->opts_sym.emplace(name.view(), std::move(literal.value));
}

void prepare_expr(Context_t *ctx, const Pos_t &) {
    ctx->branch_addrs.push();
}

void
generate_rtvar_segment(Context_t *ctx, const Token_t &token, bool is_first) {
    // for first path segments
    if (is_first) {
        generate_rtvar<PushThisFrag_t>(ctx, token);
        note_optimization_point(ctx, false);
    }

    // top level rtvar path string
    auto &rtvar_string = ctx->rtvar_strings.back();

    // process builtin variables
    switch (token) {
    case LEX2::BUILTIN_FIRST:
        generate<PushValFirst_t>(ctx, rtvar_string.str(), token.pos);
        break;
    case LEX2::BUILTIN_INNER:
        generate<PushValInner_t>(ctx, rtvar_string.str(), token.pos);
        break;
    case LEX2::BUILTIN_LAST:
        generate<PushValLast_t>(ctx, rtvar_string.str(), token.pos);
        break;
    case LEX2::BUILTIN_INDEX:
        generate<PushValIndex_t>(ctx, rtvar_string.str(), token.pos);
        break;
    case LEX2::BUILTIN_COUNT:
        generate<PushValCount_t>(ctx, rtvar_string.str(), token.pos);
        break;
    case LEX2::BUILTIN_PARENT:
        throw std::runtime_error(__PRETTY_FUNCTION__ + std::string("-parent"));
    case LEX2::BUILTIN_THIS:
        throw std::runtime_error(__PRETTY_FUNCTION__ + std::string("-this"));
    case LEX2::BUILTIN_ERROR:
        if (ctx->params->isErrorFragmentEnabled()) {
            generate<PushErrorFrag_t>(ctx, true, token.pos);
            break;
        }
        // pass through
    default:
        generate<PushAttr_t>(ctx, token.str(), rtvar_string.str(), token.pos);
        break;
    }

    // this code remains valid until the runtime variables
    // will not be broken to more strings
    rtvar_string = {rtvar_string.begin(), token.view().end()};
}

void generate_rtvar_this(Context_t *ctx, const Token_t &token, bool is_first) {
    // for first path segments
    if (is_first) {
        generate_rtvar<PushThisFrag_t>(ctx, token);
        note_optimization_point(ctx, false);
    }

    // top level rtvar path string
    auto &rtvar_string = ctx->rtvar_strings.back();
    // this code remains valid until the runtime variables
    // will not be broken to more strings
    rtvar_string = {rtvar_string.begin(), token.view().end()};
}

void
generate_rtvar_parent(Context_t *ctx, const Token_t &token, bool is_first) {
    // for first path segments
    if (is_first) {
        generate_rtvar<PushThisFrag_t>(ctx, token);
        if (ctx->open_frames.top().empty()) {
            logWarning(
                ctx,
                token.pos,
                "The builtin _parent variable has crossed root boundary; "
                "converting it to _this"
            );
        }
        note_optimization_point(ctx, false);
    }

    // top level rtvar path string
    auto &rtvar_string = ctx->rtvar_strings.back();
    // remove "last" path segment
    generate<PopAttr_t>(ctx, token.pos);
    // this code remains valid until the runtime variables
    // will not be broken to more strings
    rtvar_string = {rtvar_string.begin(), token.view().end()};
}

void generate_local_rtvar(Context_t *ctx, const Token_t &token) {
    generate_var(ctx, token);
    note_optimization_point(ctx, false);
    logWarning(
        ctx,
        token.pos,
        "The runtime variable is useless; converting it to regular variable"
    );
}

} // namespace Parser
} // namespace Teng

