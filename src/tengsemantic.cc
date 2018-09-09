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

#include "tengsyntax.hh"
#include "tengprogram.h"
#include "tengformatter.h"
#include "tengparserfrag.h"
#include "tengdictionary.h"
#include "tengcontenttype.h"
#include "tengconfiguration.h"
#include "tengsemantic.h"

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
        break;

    case LEX2::LT_DIGRAPH:
    case LEX2::LE_DIGRAPH:
    case LEX2::GT_DIGRAPH:
    case LEX2::GE_DIGRAPH:
    case LEX2::EQ_DIGRAPH:
    case LEX2::NE_DIGRAPH:
    case LEX2::AND_TRIGRAPH:
    case LEX2::OR_DIGRAPH:
    case LEX2::CASE:
    case LEX2::IDENT:
    case LEX2::VAR:
        ctx->program->emplace_back<Set_t>(var);
        break;

    default:
        logError(
            ctx,
            var.pos,
            "Invalid variable identifier: ident=" + var.symbol_view
            + ", token=" + l2_token_name(var.id));
        break;
    }
}

/** Generates var instruction.
 */
void generate_var_impl(Context_t *ctx, const Variable_t &var) {
    // generate var instruction
    switch (var.id) {
    case LEX2::BUILTIN_FIRST:
        ctx->program->emplace_back<FragFirst_t>(var);
        break;
    case LEX2::BUILTIN_INNER:
        ctx->program->emplace_back<FragInner_t>(var);
        break;
    case LEX2::BUILTIN_LAST:
        ctx->program->emplace_back<FragLast_t>(var);
        break;
    case LEX2::BUILTIN_INDEX:
        ctx->program->emplace_back<FragIndex_t>(var);
        break;
    case LEX2::BUILTIN_COUNT:
        ctx->program->emplace_back<FragCnt_t>(var);
        break;

    case LEX2::BUILTIN_PARENT:
    case LEX2::BUILTIN_THIS:
        // TODO(burlog): warning asi neni dobre, co takhle jsonify(_this), i
        // kdyz fakt to ma nejakej smysl?
        logWarning(
            ctx,
            var.pos,
            "Using builtin identifiers might cause undefined behaviour: "
            "var=" + var.symbol_view
        );
        ctx->program->emplace_back<Var_t>(var, true);
        break;

    case LEX2::IDENT:
        if (var.ident.name().front() == '_') {
            logWarning(
                ctx,
                var.pos,
                "The variable names starting with an underscore are reserved, "
                "and might cause undefined behaviour in future: var="
                + var.symbol_view
            );
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
    case LEX2::CASE:
    case LEX2::VAR:
        ctx->program->emplace_back<Var_t>(var, true);
        break;

    default:
        logError(
            ctx,
            var.pos,
            "Invalid variable identifier: ident=" + var.symbol_view
            + ", token=" + l2_token_name(var.id));
        break;
    }
}

/** Local variables has scalar offsets.
 */
void resolve_local_var(Context_t *ctx, Variable_t &var_sym) {
    var_sym.frame_offset = 0;
    var_sym.frag_offset = 0;
}

/** Attemps resolve relative variable in any open frames in reverse order. It
 * does not _ANY_ check that variable is relative. The examples explains
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
void resolve_relative_var(Context_t *ctx, Variable_t &var_sym) {
    // try resolve relative variable path in any open frames
    auto irframe = ctx->open_frames.rbegin();
    auto erframe = ctx->open_frames.rend();
    for (; irframe != erframe; ++irframe) {

        // does open fragments in current frame contain our ident
        auto irel_start_at = irframe->resolve_relative(var_sym.ident);
        if (irel_start_at != irframe->end()) {
            Identifier_t abs_ident(false);

            // if so copy path from root to the first ident segment to new ident
            auto ifrag = irframe->begin();
            for (; ifrag != irel_start_at; ++ifrag)
                abs_ident.push_back(ifrag->name);

            // then copy all ident segments to new ident
            for (auto &segment: var_sym.ident)
                abs_ident.push_back(segment);

            // and break the search (and return absolute var)
            auto path_size = abs_ident.size() - 1;
            var_sym.frame_offset = irframe - ctx->open_frames.rbegin();
            var_sym.frag_offset = irframe->size() - path_size;
            var_sym.ident = std::move(abs_ident);
            return;
        }
    }
}

/** Absolute vars can refer to variables in any open frame and open fragment
 * but not to closed one. Such absolute variable is left untouched.
 */
void resolve_abs_var(Context_t *ctx, Variable_t &var_sym) {
    // find frame where list of open frags is long enough (in reverse order)
    auto irframe = ctx->open_frames.rbegin();
    auto erframe = ctx->open_frames.rend();
    for (; irframe != erframe; ++irframe) {
        auto path_size = var_sym.ident.size() - 1;

        // if var path size is less than open frags count we found the frame
        if (path_size <= irframe->size()) {
            var_sym.frame_offset = irframe - ctx->open_frames.rbegin();
            var_sym.frag_offset = irframe->size() - path_size;
            return;
        }
    }
}

/** Generates runtime variable instructions for desired variable.
 */
void generate_rtvar(Context_t *ctx, const Variable_t &var_sym) {
    var_sym.ident.is_absolute()
        ? generate<PushRootFrag_t>(ctx, var_sym.pos)
        : generate<PushThisFrag_t>(ctx, var_sym.pos);
    for (auto &segment: var_sym.ident)
        generate<PushAttr_t>(ctx, segment.str(), var_sym.pos);
    generate<Repr_t>(ctx, var_sym.pos);
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

void note_expr_start_point(Context_t *ctx) {
    // TODO(burlog): check that we don't recurred into another expression
    ctx->expr_start_point = {
        ctx->pos(),
        static_cast<int32_t>(ctx->program->size()),
    };
}

void note_optimization_point(Context_t *ctx, bool optimizable) {
    auto addr = int32_t(ctx->program->size()) - 1;
    ctx->optimization_points.push({addr, optimizable});
}

void generate_var(Context_t *ctx, Variable_t var) {
    // instruction for local variables can be generated immediately
    if (var.ident.is_local()) {
        resolve_local_var(ctx, var);
        return generate_var_impl(ctx, var);
    }

    // absolute variables have to be resolved prior to instruction generation
    if (var.ident.is_absolute()) {
        resolve_abs_var(ctx, var);
        if (var.offsets_are_valid())
            return generate_var_impl(ctx, var);
        // resolution failed so convert scalar variable to runtime
        return generate_rtvar(ctx, var);
    }

    // relative variables have to be resolved prior to instruction generation
    resolve_relative_var(ctx, var);
    if (var.offsets_are_valid())
        return generate_var_impl(ctx, var);
    // resolution failed so convert scalar variable to runtime
    return generate_rtvar(ctx, var);
}

void prepare_root_variable(Context_t *ctx, const Token_t &token) {
    ctx->var_sym = Variable_t(token, Identifier_t(false));
}

void prepare_this_variable(Context_t *ctx, const Token_t &token) {
    Identifier_t result(false);
    for (auto &frag: ctx->open_frames.top())
        result.push_back(frag.name);
    ctx->var_sym = Variable_t(token, std::move(result));
}

void prepare_parent_variable(Context_t *ctx, const Token_t &token) {
    Identifier_t result(false);
    if (!ctx->open_frames.top().empty()) {
        for (auto i = 0lu; i < ctx->open_frames.top().size() - 1; ++i)
            result.push_back(ctx->open_frames.top()[i].name);
    } else logWarning(ctx, token.pos, "The _parent violates the root boundary");
    ctx->var_sym = Variable_t(token, std::move(result));
}

void generate_print(Context_t *ctx) {
    // get current program size
    auto prgsize = ctx->program->size();

    // underflow protect -> no optimalization can be peformed for now
    if (prgsize < 3)
        return ctx->program->emplace_back<Print_t>(ctx->pos());

    // attempt to optimize consecutive print instrs to one merged
    if ((*ctx->program)[prgsize - 1].opcode() != OPCODE::VAL)
        return ctx->program->emplace_back<Print_t>(ctx->pos());
    if ((*ctx->program)[prgsize - 2].opcode() != OPCODE::PRINT)
        return ctx->program->emplace_back<Print_t>(ctx->pos());
    if ((*ctx->program)[prgsize - 3].opcode() != OPCODE::VAL)
        return ctx->program->emplace_back<Print_t>(ctx->pos());

    // optimalize sequence of VAL, PRINT, VAL, PRINT to single VAL, PRINT pair
    auto &first_val = (*ctx->program)[prgsize - 3].as<Val_t>();
    auto &second_val = (*ctx->program)[prgsize - 1].as<Val_t>();
    first_val.value.append_str(second_val.value);

    // delete last VAL instruction (optimized out)
    ctx->program->pop_back();
}

void open_frag(Context_t *ctx, const Pos_t &pos, IVariable_t frag) {
    // if symbol is invalid then create frag instruction with empty name that
    // is used as marker for close_frag() function to discarting frag content
    if (frag.ident.empty() || frag.invalid) {
        logError(
            ctx,
            frag.pos,
            std::string(frag.invalid? "Invalid": "Empty")
            + " fragment identifier; discarding fragment block content"
        );
        ctx->open_frames.top().open_frag({}, ctx->program->size(), false);
        ctx->program->emplace_back<Frag_t>(std::string(), pos);
        return;
    }

    // for open_frag purposes local and absolute variable are sufficient
    // but the relative ones should be resolved before we open any new frag
    if (frag.ident.is_relative()) resolve_relative_var(ctx, frag);

    // create new open-fragments frame if identifier is absolute
    auto i = 0lu;
    auto *frame = &ctx->open_frames.top();
    if (frag.ident.is_absolute()) {
        // left-trim frag ident == make it relative to current open fragment
        // or open new frame if absolute ident can't be made relative
        if (!frame->is_prefix_of(frag.ident)) {
            frame = &ctx->open_frames.open_frame();
            ctx->program->emplace_back<Frame_t>(pos);
        } else i = frame->size();
    }

    // open as many fragments as needed
    for (auto first_index = i; i < frag.ident.size(); ++i) {
        bool auto_close = i != first_index;
        frame->open_frag(frag.ident[i], ctx->program->size(), auto_close);
        ctx->program->emplace_back<Frag_t>(frag.ident[i].str(), pos);
    }
}

void close_frag(Context_t *ctx, const Pos_t &pos, const Pos_t &inv_pos) {
    // tell the user that there are some excessive tokens in teng_fragment_close
    if (inv_pos) {
        logError(
            ctx,
            inv_pos,
            "Excessive tokens in <?teng endfrag ...?>; "
            "discarding fragment block content"
        );
    }

    // pop as many fragments as needed
    for (; !ctx->open_frames.top().empty();) {
        // close fragment
        auto frag = ctx->open_frames.top().close_frag();

        // create end-frag instruction and take fragment subprogram length
        ctx->program->emplace_back<EndFrag_t>(pos);
        auto frag_routine_length = ctx->program->size() - frag.addr - 1;

        // take references to instructions after push_back that invalidates them
        auto &frag_instr = (*ctx->program)[frag.addr].as<Frag_t>();
        auto &endfrag_instr = ctx->program->back().as<EndFrag_t>();

        // open frag instr contains offset of close frag instr and vice versa
        frag_instr.endfrag_offset = frag_routine_length;
        endfrag_instr.openfrag_offset = -frag_routine_length;

        // if fragment has invalid name discard all code up to open instruction
        if (inv_pos || frag.name.empty() || frag_instr.name.empty())
            ctx->program->erase_from(frag.addr);

        // close frame if is empty
        if (ctx->open_frames.top().empty()) {
            if (ctx->open_frames.size() > 1) {
                ctx->open_frames.close_frame();
                ctx->program->emplace_back<EndFrame_t>(pos);
            }
        }

        // if fragment hasn't been created as part of auto fragments then stop
        if (!frag.auto_close) return;
    }
    logError(ctx, pos, "Closing frag requested but no one opened!");
}

void optimize_expr(Context_t *ctx, uint32_t arity, bool lazy_evaluated) {
    // take the address of the first expression argument instruction
    bool optimizable = true;
    int32_t args_point = ctx->program->size() - 1;
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
            std::cerr << "$$$$ optimized to = " << result << '\n' << std::endl;
            auto pos = (*ctx->program)[args_point].pos();
            ctx->program->erase_from(args_point);
            generate_val(ctx, pos, std::move(result));

        } else optimizable = false;
        if (!optimizable) std::cerr << "$$$$ can't optimize" << std::endl;
    } else std::cerr << "$$$$ unoptimizable" << std::endl;

    // each expression has a value that plays its role in further optimization
    // if optimization fails then it is ok to replace it with one imaginary
    // value at current address because it is marked as unoptimizable
    note_optimization_point(ctx, optimizable);
}

void generate_val(Context_t *ctx, const Pos_t &pos, Value_t value) {
    ctx->program->emplace_back<Val_t>(std::move(value), pos);
}

void generate_dict_lookup(Context_t *ctx, const Token_t &token) {
    // find item in dictionary
    if (auto *item = ctx->dict->lookup(token.str()))
        return ctx->program->emplace_back<Val_t>(*item, token.pos);

    // find item in param/config dictionary
    if (auto *item = ctx->params->lookup(token.str()))
        return ctx->program->emplace_back<Val_t>(*item, token.pos);

    // use ident as result value
    auto msg = "Dictionary item '" + token.view() + "' was not found";
    logError(ctx, token.pos, msg);
    ctx->program->emplace_back<Val_t>(token.view(), token.pos);
}

void discard_expr(Context_t *ctx) {
    // verify address of initial instruction of last expr
    if (ctx->expr_start_point.addr == -1)
        throw std::runtime_error(__PRETTY_FUNCTION__);

    // discard whole expression code and replace it with undefined
    ctx->program->erase_from(ctx->expr_start_point.addr);
    ctx->program->emplace_back<Val_t>(Value_t(), ctx->expr_start_point.pos);

    // fix bin/tern operators stack of addresses
    while (!ctx->branch_start_addrs.empty()) ctx->branch_start_addrs.pop();

    // tell the user that he should fix the expression
    logError(
        ctx,
        ctx->expr_start_point.pos,
        "Invalid expression, fix it please; "
        "replacing whole expression with undefined value"
    );

    // also reset optimization points
    // (yes, it clears even nested points - we don't trust it because of error)
    ctx->optimization_points.clear();

    // also include diagnostic message if there any
    ctx->expr_diag.unwind(ctx, ctx->unexpected_token);

    // reset address of last expr instruction
    ctx->expr_start_point.addr = -1;
}

void generate_tern_op(Context_t *ctx, const Token_t &token) {
    ctx->branch_start_addrs.push(ctx->program->size());
    ctx->program->emplace_back<JmpIfNot_t>(token.pos);
}

void finalize_tern_op_true_branch(Context_t *ctx, const Token_t &token) {
    // calculate jump from begin of tern op to false branch
    int32_t cond_addr = ctx->branch_start_addrs.pop();
    int32_t false_branch_offset = ctx->program->size() - cond_addr;

    // store address of jump from true branch
    ctx->branch_start_addrs.push(ctx->program->size());
    ctx->program->emplace_back<Jmp_t>(token.pos);

    // fix conditional jump offset (relative addr)
    auto &instr = (*ctx->program)[cond_addr].as<JmpIfNot_t>();
    instr.addr_offset = false_branch_offset;
}

void finalize_tern_op_false_branch(Context_t *ctx) {
    int32_t true_branch_jump_addr = ctx->branch_start_addrs.pop();
    auto tern_op_end_offset = ctx->program->size() - true_branch_jump_addr;
    auto &instr = (*ctx->program)[true_branch_jump_addr].as<Jmp_t>();
    instr.addr_offset = tern_op_end_offset;
}

void generate_query(Context_t *ctx, const Variable_t &var, bool warn) {
    var.ident.is_absolute()
        ? generate<PushRootFrag_t>(ctx, var.pos)
        : generate<PushThisFrag_t>(ctx, var.pos);
    note_optimization_point(ctx, true);
    for (auto &segment: var.ident)
        generate<PushAttr_t>(ctx, segment.str(), var.pos);

    // warn if query is name($some.var)
    if (!warn) return;
    logWarning(
        ctx,
        var.pos,
        "In query expressions the identifier should not be denoted by $ sign"
    );
}

void invalid_query(Context_t *ctx, const Pos_t &pos) {
    logDiag(ctx, pos, "Invalid variable identifier in query");
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
    ctx->load_file(iopt->value.str(), &pos);
}

void ignore_include(Context_t *ctx, const Token_t  &token) {
    if (token == LEX2::END) {
        logError(
            ctx,
            token.pos,
            "Missing filename to include; ignoring the include directive"
        );
    } else {
        logError(
            ctx,
            token.pos,
            "Invalid or excessive tokens in <?teng include ...?>; ignoring them"
        );
    }
}

void prepare_case(Context_t *ctx, const Token_t &token) {
    expr_diag(ctx, diag_code::case_option);
    generate<Push_t>(ctx, token.pos);
}

uint32_t generate_case_cmp(Context_t *ctx, Literal_t &literal) {
    // warn if there is more than one branch valid for same value
    for (auto addr: ctx->case_option_addrs) {
        auto &instr = (*ctx->program)[addr].as<Val_t>();
        if (instr.value == literal.value) {
            auto val = literal.value.string();
            logWarning(ctx, instr.pos(), "Duplicit case operand: " + val);
            logWarning(ctx, literal.pos, "Next seen here");
            break;
        }
    }

    // generate instructions
    generate<Stack_t>(ctx, 0, literal.pos);
    ctx->case_option_addrs.push_back(ctx->program->size());
    generate<Val_t>(ctx, std::move(literal.value), literal.pos);
    generate<EQ_t>(ctx, literal.pos);
    return 0;
}

void update_case_jmp(Context_t *ctx, const Token_t &token, uint32_t alts) {
    expr_diag(ctx, diag_code::case_option_branch);
    for (; alts; --alts) {
        auto addr = ctx->branch_start_addrs.pop();
        auto &instr = (*ctx->program)[addr].as<Or_t>();
        instr.addr_offset = ctx->program->size() - addr - 1;
    }
    ctx->branch_start_addrs.push(ctx->program->size());
    generate<JmpIfNot_t>(ctx, token.pos);
}

uint32_t
generate_case_next(Context_t *ctx, Literal_t &literal, uint32_t arity) {
    ctx->branch_start_addrs.push(ctx->program->size());
    generate<Or_t>(ctx, literal.pos);
    generate_case_cmp(ctx, literal);
    return arity + 1;
}

void finalize_case_branch(Context_t *ctx, const Token_t &token) {
    auto branch_if_addr = ctx->branch_start_addrs.pop();
    auto &instr = (*ctx->program)[branch_if_addr].as<JmpIfNot_t>();
    instr.addr_offset = ctx->program->size() - branch_if_addr;
    ctx->branch_start_addrs.push(ctx->program->size());
    generate<Jmp_t>(ctx, token.pos);
}

NAryExpr_t finalize_case(Context_t *ctx, const Token_t &token, uint32_t arity) {
    // update addr offsets of jmp instruction at the end of all branches
    for (auto tmp_arity = arity; --tmp_arity;) {
        auto branch_end_addr = ctx->branch_start_addrs.pop();
        auto &instr = (*ctx->program)[branch_end_addr].as<Jmp_t>();
        instr.addr_offset = ctx->program->size() - branch_end_addr - 1;
    }

    // generate instruction that remove case value from prg stack
    generate<Pop_t>(ctx, token.pos);

    // clear stored option addresses
    ctx->case_option_addrs.clear();

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
open_format(Context_t *ctx, const Pos_t &pos, const FormatOptions_t &opts) {
    auto resolve_mode_id = [&] (auto iopt) {
        // report invalid syntax
        if (opts.invalid) {
            logError(
                ctx,
                pos,
                "Excessive or invalid tokens in <?teng format ...?> directive; "
                "Formatting block has no effect"
            );
            return Formatter_t::MODE_COPY_PREV;
        }

        // ensure that space option exists
        if (iopt == opts.end()) {
            logError(
                ctx,
                pos,
                "Formatting block has no effect; option 'space' is missing"
            );
            return Formatter_t::MODE_COPY_PREV;
        }

        // ensure that space has value
        if (iopt->value.empty()) {
            logError(
                ctx,
                pos,
                "Formatting block has no effect; option 'space' is empty"
            );
            return Formatter_t::MODE_COPY_PREV;
        }

        // check space option
        Formatter_t::Mode_t mode_id = resolveFormat(iopt->value);
        if (mode_id == Formatter_t::MODE_INVALID) {
            logError(
                ctx,
                pos,
                "Unsupported value '" + iopt->value + "'"
                "of 'space' formatting option"
            );
            return Formatter_t::MODE_COPY_PREV;
        }
        return mode_id;
    };

    // generate format instruction
    auto iopt = opts.find("space");
    generate<Format_t>(ctx, resolve_mode_id(iopt), pos);
}

void close_format(Context_t *ctx, const Pos_t &pos, const Pos_t &inv_pos) {
    generate<EndFormat_t>(ctx, pos);
    if (inv_pos) {
        logError(
            ctx,
            inv_pos,
            "Ignoring invalid or excessive tokens in "
            "<?teng endformat ...?> directive'"
        );
    }
}

void open_ctype(Context_t *ctx, const Pos_t &pos, const Literal_t &type) {
    if (type.value.is_string_like()) {
        // push new ctype instruction
        auto *desc = ContentType_t::find(type.value.string());
        generate<CType_t>(ctx, desc, pos);
        if (ctx->program->back().as<CType_t>().ctype)
            return;

        // log invalid conent type name
        logError(
            ctx,
            pos,
            "Invalid content type '" + type.value.string() + "'"
            "; using top instead"
        );
        return;
    } else generate<CType_t>(ctx, nullptr, pos);

    // log invalid teng_ctype_open directive
    logError(
        ctx,
        pos,
        "Invalid or excessive tokens in <?teng ctype ...?> directive"
        "; using top content type instead"
    );
}

void close_ctype(Context_t *ctx, const Pos_t &pos, const Pos_t &inv_pos) {
    generate<EndCType_t>(ctx, pos);
    if (inv_pos) {
        logError(
            ctx,
            inv_pos,
            "Ignoring invalid or excessive tokens in "
            "<?teng endctype ...?> directive'"
        );
    }
}

void generate_if(Context_t *ctx, const Token_t &token) {
    ctx->branch_start_addrs.push(ctx->program->size());
    generate<JmpIfNot_t>(ctx, token.pos);
    diag_code_type diag_code_value = token == LEX2::IF
        ? diag_code::if_branch
        : diag_code::elif_branch;
    expr_diag(ctx, diag_code_value);
}

void finalize_if(Context_t *ctx, int32_t shift) {
    // TODO(burlog): optimazile if

    // calculate real jump address for last elif/else branch
    auto branch_addr = ctx->branch_start_addrs.pop();
    switch ((*ctx->program)[branch_addr].opcode()) {
    case OPCODE::JMP: {
        auto &instr = (*ctx->program)[branch_addr].as<Jmp_t>();
        instr.addr_offset = ctx->program->size() - branch_addr - shift;
        break;
    }
    case OPCODE::JMPIFNOT: {
        auto &instr = (*ctx->program)[branch_addr].as<JmpIfNot_t>();
        instr.addr_offset = ctx->program->size() - branch_addr - shift;
        break;
    }
    default:
        throw std::runtime_error(__PRETTY_FUNCTION__);
    }

    // remove last branch diagnostic code
    ctx->expr_diag.pop();
}

void generate_else(Context_t *ctx, const Token_t &token) {
    ctx->branch_start_addrs.push(ctx->program->size());
    generate<Jmp_t>(ctx, token.pos);
    expr_diag(ctx, diag_code::else_branch, false);
}

void set_var(Context_t *ctx, IVariable_t var) {
    // ensure valid variable name
    if (var.ident.empty() || var.invalid) {
        logError(ctx, var.pos, "Invalid variable identifier; it won't be set");
        return;
    }

    // instruction for local variables can be generated immediately
    if (var.ident.is_local()) {
        resolve_local_var(ctx, var);
        return set_var_impl(ctx, var);
    }

    // absolute variables have to be resolved prior to instruction generation
    if (var.ident.is_absolute()) {
        resolve_abs_var(ctx, var);
        if (var.offsets_are_valid())
            return set_var_impl(ctx, var);

        // resolution failed so report error
        return logError(
            ctx,
            var.pos,
            "Variable '" + var.ident.str()
            + "' does not match any open fragment in any open frame"
        );
    }

    // relative variables have to be resolved prior to instruction generation
    resolve_relative_var(ctx, var);
    if (var.offsets_are_valid())
        return set_var_impl(ctx, var);

    // resolution failed so report error
    return logError(
        ctx,
        var.pos,
        "Variable '" + var.ident.str()
        + "' does not match any open fragment in any open frame"
    );
}

void generate_rtvar_index(Context_t *ctx, const Token_t &token) {
    generate<PushAttrAt_t>(ctx, token.pos);

    // remove optimization point of index expression because it breaks
    // "unarity" of rtvar expression and expression optimization routine pops
    // too less optimization points hence it starts from wrong address
    auto optimizable = ctx->optimization_points.top().optimizable;
    ctx->optimization_points.pop();
    ctx->optimization_points.top().optimizable &= optimizable;
}

void
generate_regex(Context_t *ctx, const Token_t &token, const Token_t &regex) {
    // parse regex (split pattern and flags)
    Regex_t regex_value;
    auto i = regex.view().size() - 1;
    for (; regex.view()[i] != '/'; ++i) {
        switch (regex.view()[i]) {
        case 'i': regex_value.flags.ignore_case = true;
        case 'I': regex_value.flags.ignore_case = false;
        case 'g': regex_value.flags.global = true;
        case 'G': regex_value.flags.global = false;
        case 'm': regex_value.flags.multiline = true;
        case 'M': regex_value.flags.multiline = false;
        default:
            logWarning(
                ctx,
                regex.pos,
                "Ignoring invalid regex flag '"
                + std::string(1, regex.view()[i])
                + "'"
                );
        }
    }
    regex_value.pattern = {regex.view().data() + 1, regex.view().data() + i};

    // generate instructin for parsed regex
    ctx->program->emplace_back<RegexMatch_t>(regex_value, regex.pos);
    if (token == LEX2::STR_NE)
        ctx->program->emplace_back<Not_t>(token.pos);
}

/** Generates instructions implementing the function call.
 */
uint32_t generate_func(Context_t *ctx, const Token_t &name, uint32_t nargs) {
    // be optimal for unescape($variable)
    // if last instr. is VAR and should be escaped
    // unescaping a single variable -- change escaping status of that variable
    if ((name.view() == "unescape") && (nargs == 1)) {
        if (ctx->program->back().opcode() == OPCODE::VAR) {
            auto &instr = ctx->program->back().as<Var_t>();
            if (instr.escape) {
                instr.escape = false;
                return nargs;
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

} // namespace Parser
} // namespace Teng

