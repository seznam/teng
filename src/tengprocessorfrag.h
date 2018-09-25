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
            "Variable '" + ctx->frames.path(instr) + "' is undefined"
        );
        return Result_t();
    }

    // check whether we have to escape variable
    if (ctx->cfg.isAlwaysEscapeEnabled()) {
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
        // TODO(burlog): path
        logWarning(
            *ctx,
            "Cannot rewrite variable '" + instr.name
            + "' which is already set by the application"
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
    auto &instr = ctx->instr->as<PushFragIndex_t>();
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

/** Pushes this frag to value stack.
 */
Result_t push_frag(RunCtxPtr_t ctx) {
    auto &instr = ctx->instr->as<PushFrag_t>();

    // we have to lookup variable in current frame and current frag
    struct VarDesc_t {string_view_t name; uint32_t frame_offset, frag_offset;};
    Value_t value = ctx->frames.get_var(VarDesc_t{instr.name, 0, 0});
    if (!value.is_undefined()) {
        logWarning(
            *ctx,
            "Identifier '" + instr.name + "' is reserved, please don't use it"
        );
        return value;
    }

    // if there is no variable of such name return appropriate frag
    return Result_t(
        ctx->frames_ptr->frag(instr.frame_offset, instr.frag_offset)
    );
}

/** If current value on value stack is fragment or item of list that is
 * fragment then the value for name stored in instruction is pushed to value
 * stack.
 */
Result_t push_attr(EvalCtx_t *ctx, GetArg_t get_arg) {
    auto arg = get_arg();
    auto &instr = ctx->instr->as<PushAttr_t>();
    auto result = ctx->frames_ptr->frag_attr(arg, instr.name);
    if (!ctx->log_suppressed && result.is_undefined())
        // what about attribute of this some frag not defined
        // TODO(burlog): path
        logWarning(*ctx, "Variable '" + instr.name + "' is undefined");
    return result;
}

/** Push frag attribute to value stack.
 */
Result_t push_attr_at(EvalCtx_t *ctx, GetArg_t get_arg) {
    auto index = get_arg();
    auto arg = get_arg();
    auto result = ctx->frames_ptr->value_at(arg, index);
    if (!ctx->log_suppressed && result.is_undefined())
        // what about attribute of this some frag not defined
        // TODO(burlog): log text
        logWarning(*ctx, "Variable '" + index.printable() + "' is undefined");
    return result;
}

Result_t pop_attr(EvalCtx_t *ctx, GetArg_t get_arg) {
    // TODO(burlog): udelej to!
    throw std::runtime_error(__PRETTY_FUNCTION__);
}

/** 
 */
Result_t repr(RunCtxPtr_t ctx, GetArg_t get_arg) {
    auto arg = get_arg();
    return Result_t(ctx->escaper.escape(ctx->frames_ptr->repr(arg).printable()));
}

/** 
 */
Result_t repr_jsonify(RunCtxPtr_t ctx, GetArg_t get_arg) {
    auto arg = get_arg();

    // // ensure type stored in arg
    // if (!arg.is_pointer()) {
    //     logError(*ctx, "Expected pointer to fragment or fragment value");
    //     return arg;
    // }

    // process value
    std::stringstream os;
    // switch (arg.template pointer_type<FRAG_PTR>()) {
    // case FRAG_PTR::NULLPTR:
    //     break;
    // case FRAG_PTR::FRAGMENT:
    //     arg.template pointer<Fragment_t>()->json(os);
    //     break;
    // case FRAG_PTR::LIST:
    //     arg.template pointer<FragmentList_t>()->json(os);
    //     break;
    // case FRAG_PTR::VALUE:
    //     arg.template pointer<FragmentValue_t>()->json(os);
    //     break;
    // }
    return Result_t(os.str());
}

/** 
 */
Result_t repr_count(RunCtxPtr_t ctx, GetArg_t get_arg) {
    auto arg = get_arg();

    // // ensure type stored in arg
    // if (!arg.is_pointer()) {
    //     logError(*ctx, "Expected pointer to fragment or fragment value");
    //     return arg;
    // }

    // // process value
    // switch (arg.template pointer_type<FRAG_PTR>()) {
    // case FRAG_PTR::NULLPTR:
    //     return Result_t("null");
    // case FRAG_PTR::FRAGMENT:
    //     return Result_t(1);
    // case FRAG_PTR::LIST:
    //     return Result_t(arg.template pointer<FragmentList_t>()->size());
    // case FRAG_PTR::VALUE:
    //     if (auto *ptr = arg.template pointer<FragmentValue_t>()) {
    //         // if (auto *list = ptr->getNestedFragments())
    //         //     return Result_t(list->size());
    //         return Result_t(1);
    //     }
    //     break;
    // }
    throw std::runtime_error(__PRETTY_FUNCTION__);
}

/** 
 */
Result_t repr_type(RunCtxPtr_t ctx, GetArg_t get_arg) {
    auto arg = get_arg();

    // // ensure type stored in arg
    // if (!arg.is_pointer()) {
    //     logError(*ctx, "Expected pointer to fragment or fragment value");
    //     return arg;
    // }

    // // process value
    // switch (arg.template pointer_type<FRAG_PTR>()) {
    // case FRAG_PTR::NULLPTR:
    //     return Result_t("null");
    // case FRAG_PTR::FRAGMENT:
    //     return Result_t("frag");
    // case FRAG_PTR::LIST:
    //     return Result_t("list");
    // case FRAG_PTR::VALUE:
    //     if (auto *ptr = arg.template pointer<FragmentValue_t>()) {
    //         return Result_t("xxx");
    //         // return ptr->getNestedFragments()
    //         //     ? Result_t("valuelist")
    //         //     : Result_t("value");
    //     }
    //     break;
    // }
    throw std::runtime_error(__PRETTY_FUNCTION__);
}

/** 
 */
Result_t repr_defined(RunCtxPtr_t ctx, GetArg_t get_arg) {
    throw std::runtime_error("not implemented yet");
}

/** 
 */
Result_t repr_exists(EvalCtx_t *ctx, GetArg_t get_arg) {
    auto arg = get_arg();
    return ctx->frames_ptr->exists(arg);
}

/** 
 */
Result_t repr_isempty(RunCtxPtr_t ctx, GetArg_t get_arg) {
    throw std::runtime_error("not implemented yet");
}

} // namespace exec
} // namespace Teng

#endif /* TENGPROCESSORFRAG_H */

