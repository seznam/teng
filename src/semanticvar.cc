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
#include "logging.h"
#include "program.h"
#include "configuration.h"
#include "parsercontext.h"
#include "semanticexpr.h"
#include "semanticvar.h"

#ifdef DEBUG
#include <iostream>
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

/** Local variable has scalar offsets.
 */
VarOffset_t resolve_local_var(Context_t *) {
    return {0, 0};
}

/** Generates runtime variable path instructions for desired variable.
 *
 * Assumption: var_sym.ident.size() > 1.
 */
void
generate_auto_rtvar_path(
    Context_t *ctx,
    const Variable_t &var_sym,
    const char *path_end,
    bool gen_repr
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
void
generate_auto_rtvar(Context_t *ctx, const Variable_t &var_sym, bool gen_repr) {
    // relative variables
    if (var_sym.ident.is_relative()) {
        generate<PushThisFrag_t>(ctx, var_sym.pos);
        auto path_end = var_sym.view().begin();
        return generate_auto_rtvar_path(ctx, var_sym, path_end, gen_repr);
    }

    // absolute variables - no open fragments
    if (ctx->open_frames.top().empty()) {
        generate<PushRootFrag_t>(ctx, uint16_t(0), var_sym.pos);
        auto path_end = var_sym.view().begin();
        return generate_auto_rtvar_path(ctx, var_sym, path_end, gen_repr);
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
        return generate_auto_rtvar_path(ctx, rel_var, path_end, gen_repr);
    }
}

/** Generates instructions implementing runtime variable this.
 */
void generate_rtvar_this_impl(Context_t *ctx, const Token_t &token) {
    uint16_t root_offset = static_cast<uint16_t>(ctx->open_frames.top().size());
    generate<PushThisFrag_t>(ctx, root_offset, token.pos);
    note_optimization_point(ctx, true);
    ctx->rtvar_strings.emplace_back(token.view().begin(), 0);
}

} // namespace

/** Generates variable lookup.
 */
void generate_var(Context_t *ctx, Variable_t var, bool gen_repr) {
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
        return generate_auto_rtvar(ctx, var, gen_repr);
    }

    // relative variables have to be resolved prior to instruction generation
    if ((var.offset = resolve_relative_var(ctx, var))) {
        var.ident = make_absolute_ident(ctx, var);
        return generate_var_impl(ctx, var);
    }
    // resolution failed so convert scalar variable to runtime
    return generate_auto_rtvar(ctx, var, gen_repr);
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

void
generate_rtvar_segment(Context_t *ctx, const Token_t &token, bool is_first) {
    // for first path segments
    if (is_first) {
        generate_rtvar_this_impl(ctx, token);
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
        generate_rtvar_this_impl(ctx, token);
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
        generate_rtvar_this_impl(ctx, token);
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

void generate_rtvar_root(Context_t *ctx, const Token_t &token) {
    uint16_t root_offset = static_cast<uint16_t>(ctx->open_frames.top().size());
    generate<PushRootFrag_t>(ctx, root_offset, token.pos);
    note_optimization_point(ctx, true);
    ctx->rtvar_strings.push_back(token.view());
}

void ignoring_this(Context_t *ctx, const Pos_t &pos) {
    logWarning(ctx, pos, "Ignoring useless '_this' variable path segment");
}

void obsolete_dollar(Context_t *ctx, const Pos_t &pos) {
    logWarning(ctx, pos, "Don't use dollar sign here please");
}

VarOffset_t resolve_relative_var(Context_t *ctx, const Variable_t &var_sym) {
    // initialied to invalid offsets
    VarOffset_t offset;

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

VarOffset_t resolve_abs_var(Context_t *ctx, const Variable_t &var_sym) {
    // initialied to invalid offsets
    VarOffset_t offset;

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

} // namespace Parser
} // namespace Teng

