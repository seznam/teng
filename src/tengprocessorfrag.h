/*
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
 * $Id: tengprocessor.cc,v 1.15 2010-06-11 07:46:26 burlog Exp $
 *
 * DESCRIPTION
 * Teng processor executors.
 *
 * AUTHORS
 * Jan Nemec <jan.nemec@firma.seznam.cz>
 * Vaclav Blazek <blazek@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-07-07  (burlog)
 *             Extracted from tengprocessor.cc.
 */

#ifndef TENGPROCESSORFRAG_H
#define TENGPROCESSORFRAG_H

#include <sstream>

#include "tengprocessorcontext.h"

namespace Teng {
namespace exec {

/** Implementation of the variable lookup.
 */
Result_t var(RunCtxPtr_t ctx, bool escape) {
    auto &instr = ctx->instr->as<Var_t>();

    // if variable does not exist then return empty string
    Value_t value = ctx->frames.get_var(instr);
    if (value.is_undefined()) {
        logWarning(
            *ctx,
            "Variable '" + ctx->frames.path(instr)
            + "' is undefined [open_frags="
            + ctx->frames_ptr->current_path() + ", iteration="
            + std::to_string(ctx->frames_ptr->current_list_i()) + "]"
        );
        return Result_t();
    }

    // check whether we have to escape variable
    if (ctx->params.isPrintEscapeEnabled()) {
        // no escaping needed, value will be escaped prior to printing
        return value;

    } else if (ctx->params.isAlwaysEscapeEnabled()) {
        if (instr.escape && value.is_string_like())
            return Result_t(ctx->escaper.escape(value.string()));

    } else if (escape && value.is_string_like())
        return Result_t(ctx->escaper.escape(value.string()));

    // no escaping
    return value;
}

/** Set variable value.
 */
void set_var(RunCtxPtr_t ctx, GetArg_t get_arg) {
    auto &instr = ctx->instr->as<Set_t>();
    Value_t arg = get_arg();

    if (!ctx->frames.set_var(instr, std::move(arg))) {
        logWarning(
            *ctx,
            "Cannot rewrite variable '" + ctx->frames.path(instr)
            + "' which is already set by the application; nothing set"
            + " [open_frags="
            + ctx->frames_ptr->current_path() + ", iteration="
            + std::to_string(ctx->frames_ptr->current_list_i()) + "]"
        );
    }
}

/** Opens new frame.
 */
void open_frame(RunCtxPtr_t ctx) {
    ctx->frames.open_frame();
}

/** Closes the most recent frame.
 */
void close_frame(RunCtxPtr_t ctx) {
    ctx->frames.close_frame();
}

/** Open new fragment in current frame if there is at least one frag of such
 * name or jump over frag block.
 */
int32_t open_frag(RunCtxPtr_t ctx) {
    auto &instr = ctx->instr->as<OpenFrag_t>();
    return ctx->frames.open_frag(instr.name)
         ? 0
         : instr.close_frag_offset;
}

/** Pop frag from top of the stack of open frags.
 */
int32_t close_frag(RunCtxPtr_t ctx) {
    auto &instr = ctx->instr->as<CloseFrag_t>();
    return ctx->frames.next_frag()
        ? instr.open_frag_offset
        : 0;
}

/** Returns the number of fragments in current opened fragment list.
 */
Result_t frag_count(RunCtxPtr_t ctx) {
    auto &instr = ctx->instr->as<PushFragCount_t>();
    if (auto list_pos = ctx->frames.get_list_pos(instr))
        return Result_t(list_pos.size);
    logWarning(
        *ctx,
        "Can't determine '" + ctx->frames.path(instr) + "' fragment size"
    );
    return Result_t();
}

/** Returns index of current fragmnet in opened fragment list.
 */
Result_t frag_index(RunCtxPtr_t ctx, uint32_t *size = nullptr) {
    auto &instr = ctx->instr->as<PushFragIndex_t>();
    if (auto list_pos = ctx->frames.get_list_pos(instr))
        return Result_t(list_pos.i);
    logWarning(
        *ctx,
        "Can't determine '" + ctx->frames.path(instr) + "' fragment index"
    );
    return Result_t();
}

/** Returns true if current fragment is the first in opened fragment list.
 */
Result_t is_first_frag(RunCtxPtr_t ctx) {
    auto &instr = ctx->instr->as<PushFragFirst_t>();
    if (auto list_pos = ctx->frames.get_list_pos(instr))
        return Result_t(list_pos.i == 0);
    logWarning(
        *ctx,
        "Can't determine '" + ctx->frames.path(instr) + "' fragment index"
    );
    return Result_t(false);
}

/** Returns true if current fragment is the last in opened fragment list.
 */
Result_t is_last_frag(RunCtxPtr_t ctx) {
    auto &instr = ctx->instr->as<PushFragLast_t>();
    if (auto list_pos = ctx->frames.get_list_pos(instr))
        return Result_t((list_pos.i + 1) == list_pos.size);
    logWarning(
        *ctx,
        "Can't determine '" + ctx->frames.path(instr) + "' fragment index"
    );
    return Result_t(false);
}

/** Returns true if the current fragment is not the first neither the last.
 */
Result_t is_inner_frag(RunCtxPtr_t ctx) {
    auto &instr = ctx->instr->as<PushFragInner_t>();
    if (auto list_pos = ctx->frames.get_list_pos(instr))
        return Result_t((list_pos.i > 0) && ((list_pos.i + 1) < list_pos.size));
    logWarning(
        *ctx,
        "Can't determine '" + ctx->frames.path(instr) + "' fragment index"
    );
    return Result_t(false);
}

/** Pushes root frag to value stack.
 */
Result_t push_root_frag(EvalCtx_t *ctx) {
    auto &instr = ctx->instr->as<PushRootFrag_t>();
    return Result_t(ctx->frames_ptr->frag(0, instr.root_frag_offset));
}

/** Pushes this frag to value stack.
 */
Result_t push_this_frag(EvalCtx_t *ctx) {
    return Result_t(ctx->frames_ptr->frag(0, 0));
}

/** Pushes the frag to value stack.
 */
Result_t push_frag(RunCtxPtr_t ctx) {
    auto &instr = ctx->instr->as<PushFrag_t>();

    // we have to lookup variable in current frame and current frag
    Value_t value = ctx->frames.get_var({instr.name});
    if (!value.is_undefined()) {
        logWarning(
            *ctx,
            "Identifier '" + instr.name + "' is reserved, please don't use it"
        );
        return value;
    }

    // if there is no variable of such name then return frag at given offsets
    // note that no name is needed because the instruction is intended to
    // accessing yet open fragments
    return Result_t(
        ctx->frames_ptr->frag(instr.frame_offset, instr.frag_offset)
    );
}

/** If current value on value stack is fragment or item of list that is
 * fragment then the value for name stored in instruction is pushed to value
 * stack.
 */
template <typename Ctx_t>
Result_t push_attr(Ctx_t *ctx, GetArg_t get_arg) {
    auto arg = get_arg();

    // skip undefined values immediately
    if (arg.is_undefined())
        return Result_t();

    // attempt to get value for desired name
    auto &instr = ctx->instr->template as<PushAttr_t>();
    auto result = ctx->frames_ptr->frag_attr(arg, instr.name);
    if (result.is_undefined()) {
        auto i = ctx->frames_ptr->current_list_i();
        if (instr.path.empty()) {
            logError(
                *ctx,
                "This fragment doesn't contain any value for key '" + instr.name
                + "' [open_frags=" + ctx->frames_ptr->current_path()
                + ", iteration=" + std::to_string(i) + "]"
            );
        } else {
            logError(
                *ctx,
                "The path expression '" + instr.path + "' references fragment "
                "that doesn't contain any value for key '" + instr.name
                + "' [open_frags=" + ctx->frames_ptr->current_path()
                + ", iteration=" + std::to_string(i) + "]"
            );
        }
    }
    return result;
}

/** Push frag attribute to value stack.
 */
Result_t push_attr_at(EvalCtx_t *ctx, GetArg_t get_arg) {
    auto index = get_arg();
    auto arg = get_arg();

    // skip undefined values immediately
    if (arg.is_undefined())
        return Result_t();

    // attempt to get value for desired index
    auto &instr = ctx->instr->template as<PushAttrAt_t>();
    auto result = ctx->frames_ptr->value_at(arg, index);
    if (result.is_undefined()) {
        auto i = ctx->frames_ptr->current_list_i();
        switch (arg.type()) {
        case Value_t::tag::string:
        case Value_t::tag::string_ref:
        case Value_t::tag::undefined:
        case Value_t::tag::integral:
        case Value_t::tag::real:
        case Value_t::tag::regex:
            logWarning(
                *ctx,
                "The path expression '" + instr.path + "' references object "
                "of '" + arg.type_str() + "' type with value '"
                + arg.printable() + "' that is not subscriptable"
                + " [open_frags=" + ctx->frames_ptr->current_path()
                + ", iteration=" + std::to_string(i) + "]"
            );
            break;

        case Value_t::tag::frag_ref:
            if (index.is_string_like()) {
                logWarning(
                    *ctx,
                    "The path expression '" + instr.path + "' references "
                    "fragment that doesn't contain any value for key '"
                    + index.string()
                    + "' [open_frags=" + ctx->frames_ptr->current_path()
                    + ", iteration=" + std::to_string(i) + "]"
                );
            } else {
                logWarning(
                    *ctx,
                    "The path expression '" + instr.path + "' references "
                    "fragment which can't be subscripted by values of '"
                    + index.type_str() + "' type with value '"
                    + index.printable()
                    + "' [open_frags=" + ctx->frames_ptr->current_path()
                    + ", iteration=" + std::to_string(i) + "]"
                );
            }
            break;
        case Value_t::tag::list_ref:
            if (index.is_number()) {
                logWarning(
                    *ctx,
                    "The index '" + index.printable() + "' is out of valid "
                    "range <0, "
                    + std::to_string(arg.as_list_ref().ptr->size())
                    + ") of the fragments list referenced by this path "
                    "expression " + instr.path
                    + " [open_frags=" + ctx->frames_ptr->current_path()
                    + ", iteration=" + std::to_string(i) + "]"
                );
            } else {
                logWarning(
                    *ctx,
                    "The path expression '" + instr.path + "' references "
                    "fragment lists which can't be subscripted by values "
                    "of '" + index.type_str() + "' type with value '"
                    + index.printable()
                    + "' [open_frags=" + ctx->frames_ptr->current_path()
                    + ", iteration=" + std::to_string(i) + "]"
                );
            }
            break;
        }
    }
    return result;
}

/** Pops the last segment of path.
 */
Result_t pop_attr(EvalCtx_t *ctx, GetArg_t get_arg) {
    logError(*ctx, "Not implemented yet - _parent segment ignored");
    return get_arg();
}

/** Applies current escaping on the string values and other values left
 * untouched.
 */
Result_t repr(RunCtxPtr_t ctx, GetArg_t get_arg) {
    auto arg = get_arg();
    switch (arg.type()) {
    case Value_t::tag::string:
    case Value_t::tag::string_ref:
        // no escaping needed, value will be escaped prior to printing
        return ctx->params.isPrintEscapeEnabled()
            ? arg
            : Result_t(ctx->escaper.escape(arg.string()));
    case Value_t::tag::undefined:
    case Value_t::tag::integral:
    case Value_t::tag::real:
    case Value_t::tag::regex:
    case Value_t::tag::frag_ref:
    case Value_t::tag::list_ref:
        return arg;
    };
}

/** Converts value to json.
 */
Result_t repr_jsonify(RunCtxPtr_t ctx, GetArg_t get_arg) {
    --ctx->log_suppressed;
    auto arg = get_arg();
    std::stringstream o;
    arg.json(o);
    return Result_t(o.str());
}

/** Returns the number of elements in the fragment list.
 */
Result_t repr_count(RunCtxPtr_t ctx, GetArg_t get_arg) {
    --ctx->log_suppressed;
    auto arg = get_arg();
    switch (arg.type()) {
    case Value_t::tag::string:
    case Value_t::tag::string_ref:
    case Value_t::tag::integral:
    case Value_t::tag::real:
    case Value_t::tag::regex:
    case Value_t::tag::undefined:
    case Value_t::tag::frag_ref:
        logWarning(
            *ctx,
            "The path expression references object of '" + arg.type_str() + "' "
            "type with value '" + arg.printable() + "' for which count() query "
            "is undefined "
            + "[open_frags=" + ctx->frames_ptr->current_path()
            + ", iteration=" + std::to_string(ctx->frames_ptr->current_list_i())
            + "]"
        );
        return Result_t();
    case Value_t::tag::list_ref:
        return Result_t(arg.as_list_ref().ptr->size());
    }
}

/** Returns the type of desired argument.
 */
Result_t repr_type(RunCtxPtr_t ctx, GetArg_t get_arg) {
    --ctx->log_suppressed;
    return Result_t(get_arg().type_str());
}

/** Returns value for all scalar values. Returns 1 if value is fragment or
 * non empty fragment list and return 0 if value is undefined or empty fragment
 * list.
 *
 * The REPR_DEFINED is incompatible with older Teng implementation because it
 * is schizophrenic. It returns:
 *
 * >>> t.generatePage(templateString="${defined(b)}", data={'b':''})['output']
 * ''
 * >>> t.generatePage(templateString="${defined($$b)}", data={'b':''})['output']
 * '1'
 */
Result_t repr_defined(RunCtxPtr_t ctx, GetArg_t get_arg) {
    --ctx->log_suppressed;
    logWarning(*ctx, "The defined() operator is deprecated");
    auto arg = get_arg();
    switch (arg.type()) {
    case Value_t::tag::string:
    case Value_t::tag::string_ref:
    case Value_t::tag::integral:
    case Value_t::tag::real:
    case Value_t::tag::regex:
        return arg;
    case Value_t::tag::undefined:
        return Result_t(0);
    case Value_t::tag::frag_ref:
        return Result_t(1);
    case Value_t::tag::list_ref:
        return Result_t(!arg.as_list_ref().ptr->empty());
    }
}

/** Returns true if object exists.
 */
Result_t repr_exists(EvalCtx_t *ctx, GetArg_t get_arg) {
    --ctx->log_suppressed;
    auto arg = get_arg();
    return ctx->frames_ptr->exists(arg);
}

/** Returns true if frag or fraglist is empty.
 */
Result_t repr_isempty(RunCtxPtr_t ctx, GetArg_t get_arg) {
    --ctx->log_suppressed;
    auto arg = get_arg();
    switch (arg.type()) {
    case Value_t::tag::string:
    case Value_t::tag::string_ref:
    case Value_t::tag::integral:
    case Value_t::tag::real:
    case Value_t::tag::regex:
    case Value_t::tag::undefined:
        logWarning(
            *ctx,
            "The path expression references object of '" + arg.type_str() + "' "
            "type with value '" + arg.printable() + "' for which isempty() "
            "query is undefined "
            + "[open_frags=" + ctx->frames_ptr->current_path()
            + ", iteration=" + std::to_string(ctx->frames_ptr->current_list_i())
            + "]"
        );
        return Result_t();
    case Value_t::tag::frag_ref:
        return Result_t(arg.as_frag_ref().ptr->empty());
    case Value_t::tag::list_ref:
        return Result_t(arg.as_list_ref().ptr->empty());
    }
}

} // namespace exec
} // namespace Teng

#endif /* TENGPROCESSORFRAG_H */

