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

#include <limits>
#include <sstream>

#include "processorcontext.h"

namespace Teng {
namespace exec {
namespace {

/** Returns string that should be added to all errors and warnings.
 */
template <typename Ctx_t>
std::string log_suffix(Ctx_t ctx) {
    std::ostringstream out;
    out << " [open_frags=" << ctx->frames_ptr->current_path()
        << ", iteration=" << ctx->frames_ptr->current_list_i()
        << "/" << ctx->frames_ptr->current_list_size()
        << "]";
    return out.str();
}

/** Shortcut for logWarning(ctx, msg + log_suffix(ctx)).
 */
template <typename Ctx_t>
void warn(Ctx_t ctx, const string_view_t &msg) {
    logWarning(*ctx, msg + log_suffix(ctx));
}

} // namespace

/** Implementation of the variable lookup.
 */
Result_t var(RunCtxPtr_t ctx, bool escape) {
    auto &instr = ctx->instr->as<Var_t>();

    // if variable does not exist then return empty string
    Value_t value = ctx->frames.get_var(instr);
    if (value.is_undefined()) {
        warn(ctx, "Variable '" + ctx->frames.path(instr) + "' is undefined");
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
    if (!ctx->frames.set_var(instr, get_arg())) {
        warn(
            ctx,
            "Cannot rewrite variable '" + ctx->frames.path(instr)
            + "' which is already set by the application; nothing set"
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

/** Open error fragment in current frame.
 */
int32_t open_error_frag(RunCtxPtr_t ctx) {
    auto &instr = ctx->instr->as<OpenErrorFrag_t>();
    auto enabled = ctx->params.isErrorFragmentEnabled();
    return enabled && ctx->frames.open_error_frag(ctx->err.getFrags())
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
    warn(ctx, "Can't determine '" + ctx->frames.path(instr) + "' frag count");
    return Result_t();
}

/** Returns the number of fragments in current opened fragment list.
 */
Result_t frag_count(RunCtxPtr_t ctx, GetArg_t get_arg) {
    auto &instr = ctx->instr->as<PushValCount_t>();
    auto arg = get_arg();
    switch (arg.type()) {
    case Value_t::tag::undefined:
        return Result_t();
    case Value_t::tag::list_ref:
        return Result_t(arg.as_list_ref().ptr->size());
    case Value_t::tag::frag_ref:
        if (ctx->frames.root_frag()->fragment() == arg.as_frag_ref().ptr)
            return Result_t(1); // (backward compatibility)
        // pass through
    default:
        warn(
            ctx,
            "The path expression '" + instr.path + "' references object "
            "of '" + arg.type_str() + "' type with value '"
            + arg.printable() + "' for which is _count builtin variable "
            + "undefined"
        );
        return Result_t();
    }
}

/** Returns index of current fragmnet in opened fragment list.
 */
Result_t frag_index(RunCtxPtr_t ctx) {
    auto &instr = ctx->instr->as<PushFragIndex_t>();
    if (auto list_pos = ctx->frames.get_list_pos(instr))
        return Result_t(list_pos.i);
    warn(ctx, "Can't determine '" + ctx->frames.path(instr) + "' frag index");
    return Result_t();
}

/** Returns index of current fragmnet in opened fragment list.
 */
Result_t frag_index(RunCtxPtr_t ctx, GetArg_t get_arg) {
    auto &instr = ctx->instr->as<PushValIndex_t>();
    auto arg = get_arg();
    switch (arg.type()) {
    case Value_t::tag::undefined:
        return Result_t();
    case Value_t::tag::list_ref:
        switch (arg.as_list_ref().ptr->size()) {
        case 1:
            return Result_t(0);
        case 0:
            warn(
                ctx,
                "The path '" + instr.path + "' references fragment list that "
                "does not contain any fragment; _index variable is undefined"
            );
            return Result_t();
        default:
            warn(
                ctx,
                "The path '" + instr.path + "' references fragment list of "
                + std::to_string(arg.as_list_ref().ptr->size()) + " fragments; "
                "_index variable is undefined"
            );
            return Result_t();
        }
    case Value_t::tag::frag_ref:
        if (ctx->frames.root_frag()->fragment() == arg.as_frag_ref().ptr)
            return Result_t(0); // (backward compatibility)
        // pass through
    default:
        warn(
            ctx,
            "The path expression '" + instr.path + "' references object "
            "of '" + arg.type_str() + "' type with value '"
            + arg.printable() + "' for which is _index builtin variable "
            + "undefined"
        );
        return Result_t();
    }
}

/** Returns true if current fragment is the first in opened fragment list.
 */
Result_t is_first_frag(RunCtxPtr_t ctx) {
    auto &instr = ctx->instr->as<PushFragFirst_t>();
    if (auto list_pos = ctx->frames.get_list_pos(instr))
        return Result_t(list_pos.i == 0);
    warn(ctx, "Can't determine '" + ctx->frames.path(instr) + "' frag index");
    return Result_t();
}

/** Returns true if current fragment is the first in opened fragment list.
 */
Result_t is_first_frag(RunCtxPtr_t ctx, GetArg_t get_arg) {
    auto &instr = ctx->instr->as<PushValFirst_t>();
    auto arg = get_arg();
    switch (arg.type()) {
    case Value_t::tag::undefined:
        return Result_t();
    case Value_t::tag::list_ref:
        switch (arg.as_list_ref().ptr->size()) {
        case 1:
            return Result_t(arg.as_list_ref().i == 0);
        case 0:
            warn(
                ctx,
                "The path '" + instr.path + "' references fragment list that "
                "does not contain any fragment; _first variable is undefined"
            );
            return Result_t();
        default:
            warn(
                ctx,
                "The path '" + instr.path + "' references fragment list of "
                + std::to_string(arg.as_list_ref().ptr->size()) + " fragments; "
                "_first variable is undefined"
            );
            return Result_t();
        }
    case Value_t::tag::frag_ref:
        if (ctx->frames.root_frag()->fragment() == arg.as_frag_ref().ptr)
            return Result_t(1); // (backward compatibility)
        // pass through
    default:
        warn(
            ctx,
            "The path expression '" + instr.path + "' references object "
            "of '" + arg.type_str() + "' type with value '"
            + arg.printable() + "' for which is _first builtin variable "
            + "undefined"
        );
        return Result_t();
    }
}

/** Returns true if current fragment is the last in opened fragment list.
 */
Result_t is_last_frag(RunCtxPtr_t ctx) {
    auto &instr = ctx->instr->as<PushFragLast_t>();
    if (auto list_pos = ctx->frames.get_list_pos(instr))
        return Result_t((list_pos.i + 1) == list_pos.size);
    warn(ctx, "Can't determine '" + ctx->frames.path(instr) + "' frag index");
    return Result_t();
}

/** Returns true if current fragment is the last in opened fragment list.
 */
Result_t is_last_frag(RunCtxPtr_t ctx, GetArg_t get_arg) {
    auto &instr = ctx->instr->as<PushValLast_t>();
    auto arg = get_arg();
    switch (arg.type()) {
    case Value_t::tag::undefined:
        return Result_t();
    case Value_t::tag::list_ref:
        switch (arg.as_list_ref().ptr->size()) {
        case 1: {
            auto i = arg.as_list_ref().i;
            auto list_size = arg.as_list_ref().ptr->size();
            return Result_t((i + 1) == list_size);
        }
        case 0:
            warn(
                ctx,
                "The path '" + instr.path + "' references fragment list that "
                "does not contain any fragment; _last variable is undefined"
            );
            return Result_t();
        default:
            warn(
                ctx,
                "The path '" + instr.path + "' references fragment list of "
                + std::to_string(arg.as_list_ref().ptr->size()) + " fragments; "
                "_last variable is undefined"
            );
            return Result_t();
        }
    case Value_t::tag::frag_ref:
        if (ctx->frames.root_frag()->fragment() == arg.as_frag_ref().ptr)
            return Result_t(1); // (backward compatibility)
        // pass through
    default:
        warn(
            ctx,
            "The path expression '" + instr.path + "' references object "
            "of '" + arg.type_str() + "' type with value '"
            + arg.printable() + "' for which is _last builtin variable "
            + "undefined"
        );
        return Result_t();
    }
}

/** Returns true if the current fragment is not the first neither the last.
 */
Result_t is_inner_frag(RunCtxPtr_t ctx) {
    auto &instr = ctx->instr->as<PushFragInner_t>();
    if (auto list_pos = ctx->frames.get_list_pos(instr))
        return Result_t((list_pos.i > 0) && ((list_pos.i + 1) < list_pos.size));
    warn(ctx, "Can't determine '" + ctx->frames.path(instr) + "' frag index");
    return Result_t();
}

/** Returns true if the current fragment is not the first neither the last.
 */
Result_t is_inner_frag(RunCtxPtr_t ctx, GetArg_t get_arg) {
    auto &instr = ctx->instr->as<PushValLast_t>();
    auto arg = get_arg();
    switch (arg.type()) {
    case Value_t::tag::undefined:
        return Result_t();
    case Value_t::tag::list_ref:
        switch (arg.as_list_ref().ptr->size()) {
        case 1: {
            auto i = arg.as_list_ref().i;
            auto list_size = arg.as_list_ref().ptr->size();
            return Result_t((i > 0) && ((i + 1) < list_size));
        }
        case 0:
            warn(
                ctx,
                "The path '" + instr.path + "' references fragment list that "
                "does not contain any fragment; _inner variable is undefined"
            );
            return Result_t();
        default:
            warn(
                ctx,
                "The path '" + instr.path + "' references fragment list of "
                + std::to_string(arg.as_list_ref().ptr->size()) + " fragments; "
                "_inner variable is undefined"
            );
            return Result_t();
        }
    case Value_t::tag::frag_ref:
        if (ctx->frames.root_frag()->fragment() == arg.as_frag_ref().ptr)
            return Result_t(0); // (backward compatibility)
        // pass through
    default:
        warn(
            ctx,
            "The path expression '" + instr.path + "' references object "
            "of '" + arg.type_str() + "' type with value '"
            + arg.printable() + "' for which is _inner builtin variable "
            + "undefined"
        );
        return Result_t();
    }
}

/** Pushes root frag (regardless of the name it can be any value) to value
 * stack.
 */
Result_t push_root_frag(EvalCtx_t *ctx) {
    auto &instr = ctx->instr->as<PushRootFrag_t>();
    return Result_t(ctx->frames_ptr->value_at(0, instr.root_frag_offset));
}

/** Pushes this frag (regardless of the name it can be any value) to value
 * stack.
 */
Result_t push_this_frag(EvalCtx_t *ctx) {
    return Result_t(ctx->frames_ptr->value_at(0, 0));
}

/** Pushes error fragment on value stack. Error fragment is stored in current
 * open fragment and create only if it does not exists.
 */
Result_t push_error_frag(RunCtxPtr_t ctx, GetArg_t get_arg) {
    auto &instr = ctx->instr->as<PushErrorFrag_t>();
    if (instr.discard_stack_value)
        get_arg();
    if (!ctx->params.isErrorFragmentEnabled())
        return Result_t();
    if (!ctx->frames.current_error_frag())
        ctx->frames.store_error_frag(ctx->err.getFrags());
    return Result_t(ctx->frames.current_error_frag());
}

/** Pushes the frag (regardless of the name it can be any value) to value stack.
 */
Result_t push_frag(RunCtxPtr_t ctx) {
    auto &instr = ctx->instr->as<PushFrag_t>();

    // we have to lookup variable in current frame and current frag
    Value_t value = ctx->frames.get_var({instr.name});
    if (!value.is_undefined()) {
        logWarning(
            *ctx,
            "The '" + instr.name + "' identifier is reserved; "
            "don't use it, please"
        );
        return value;
    }

    // if there is no variable of such name then return frag at given offsets
    // note that no name is needed because the instruction is intended to
    // accessing yet open fragments
    return Result_t(
        ctx->frames_ptr->value_at(instr.frame_offset, instr.frag_offset)
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
    std::size_t ambiguous = std::numeric_limits<std::size_t>::infinity();
    auto &instr = ctx->instr->template as<PushAttr_t>();
    auto result = ctx->frames_ptr->value_at(arg, instr.name, ambiguous);

    // attribute has been found
    if (!result.is_undefined())
        return result;

    // current fragment does not contain attribute
    if (instr.path.empty()) {
        if (ambiguous != std::numeric_limits<std::size_t>::infinity()) {
            warn(
                ctx,
                "The key '" + instr.name + "' references frament list of '"
                + std::to_string(ambiguous) + "' fragments; the expression "
                "is ambiguous"
            );
            return result;
        }
        warn(
            ctx,
            "This fragment doesn't contain any value for key '" + instr.name
            + "'"
        );
        return result;
    }

    // the expression is ambiguous
    if (ambiguous != std::numeric_limits<std::size_t>::infinity()) {
        warn(
            ctx,
            "The path expression '" + instr.path + "' references fragment list "
            "of '" + std::to_string(ambiguous) + "' fragments; "
            "the expression is ambiguous"
        );
        return result;
    }

    // attribute hasn't been found
    warn(
        ctx,
        "The path expression '" + instr.path + "' references fragment "
        "that doesn't contain any value for key '" + instr.name + "'"
    );
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
    std::size_t ambiguous = std::numeric_limits<std::size_t>::infinity();
    auto &instr = ctx->instr->template as<PushAttrAt_t>();
    auto result = ctx->frames_ptr->value_at(arg, index, ambiguous);

    // attribute has been found
    if (!result.is_undefined())
        return result;

    // the expression is ambiguous
    if (ambiguous != std::numeric_limits<std::size_t>::infinity()) {
        warn(
            ctx,
            "The path expression '" + instr.path + "' references fragment list "
            "of '" + std::to_string(ambiguous) + "' fragments; "
            "the expression is ambiguous"
        );
        return result;
    }

    // attribute hasn't been found
    switch (arg.type()) {
    case Value_t::tag::string:
    case Value_t::tag::string_ref:
    case Value_t::tag::undefined:
    case Value_t::tag::integral:
    case Value_t::tag::real:
    case Value_t::tag::regex:
        warn(
            ctx,
            "The path expression '" + instr.path + "' references object "
            "of '" + arg.type_str() + "' type with value '"
            + arg.printable() + "' that is not subscriptable"
        );
        break;

    case Value_t::tag::frag_ref:
        if (index.is_string_like()) {
            warn(
                ctx,
                "The path expression '" + instr.path + "' references "
                "fragment that doesn't contain any value for key '"
                + index.string() + "'"
            );
        } else {
            warn(
                ctx,
                "The path expression '" + instr.path + "' references "
                "fragment which can't be subscripted by values of '"
                + index.type_str() + "' type with value '"
                + index.printable() + "'"
            );
        }
        break;
    case Value_t::tag::list_ref:
        if (index.is_number()) {
            warn(
                ctx,
                "The index '" + index.printable() + "' is out of valid "
                "range <0, "
                + std::to_string(arg.as_list_ref().ptr->size())
                + ") of the fragments list referenced by this path "
                "expression '" + instr.path + "'"
            );
        } else {
            warn(
                ctx,
                "The path expression '" + instr.path + "' references "
                "fragment lists which can't be subscripted by values "
                "of '" + index.type_str() + "' type with value '"
                + index.printable() + "'"
            );
        }
        break;
    }
    return result;
}

/** Pops the last segment of path.
 */
Result_t pop_attr(EvalCtx_t *ctx, GetArg_t get_arg) {
    warn(ctx, "Not implemented yet - _parent segment ignored");
    return get_arg();
}

/** Applies current escaping on the string values and other values left
 * untouched.
 */
Result_t repr(EvalCtx_t *ctx, GetArg_t get_arg) {
    auto arg = get_arg();
    switch (arg.type()) {
    case Value_t::tag::string:
    case Value_t::tag::string_ref:
        // no escaping needed, value will be escaped prior to printing
        return ctx->params.isPrintEscapeEnabled()
            ? arg
            : Result_t(ctx->escaper_ptr->escape(arg.string()));
    case Value_t::tag::undefined:
    case Value_t::tag::integral:
    case Value_t::tag::real:
    case Value_t::tag::regex:
    case Value_t::tag::frag_ref:
    case Value_t::tag::list_ref:
        return arg;
    };
    throw std::runtime_error(__PRETTY_FUNCTION__);
}

/** Applies current escaping on the string values and other values left
 * untouched.
 */
Result_t query_repr(EvalCtx_t *ctx, GetArg_t get_arg) {
    --ctx->log_suppressed;
    return repr(ctx, get_arg);
}

/** Returns the number of elements in the fragment list.
 */
Result_t query_count(RunCtxPtr_t ctx, GetArg_t get_arg) {
    --ctx->log_suppressed;
    auto arg = get_arg();
    switch (arg.type()) {
    case Value_t::tag::frag_ref:
        if (ctx->frames.root_frag()->fragment() == arg.as_frag_ref().ptr)
            return Result_t(1); // (backward compatibility)
        // pass through
    case Value_t::tag::string:
    case Value_t::tag::string_ref:
    case Value_t::tag::integral:
    case Value_t::tag::real:
    case Value_t::tag::regex:
    case Value_t::tag::undefined:
        warn(
            ctx,
            "The path expression references object of '" + arg.type_str() + "' "
            "type with value '" + arg.printable() + "' for which count() query "
            "is undefined"
        );
        return Result_t();
    case Value_t::tag::list_ref:
        return Result_t(arg.as_list_ref().ptr->size());
    }
    throw std::runtime_error(__PRETTY_FUNCTION__);
}

/** Returns the type of desired argument.
 */
Result_t query_type(RunCtxPtr_t ctx, GetArg_t get_arg) {
    --ctx->log_suppressed;
    return Result_t(get_arg().type_str());
}

/** Returns value for all scalar values. Returns 1 if value is fragment or
 * non empty fragment list and return 0 if value is undefined or empty fragment
 * list.
 *
 * The QUERY_DEFINED is incompatible with older Teng implementation because it
 * is schizophrenic. It returns:
 *
 * >>> t.generatePage(templateString="${defined(b)}", data={'b':''})['output']
 * ''
 * >>> t.generatePage(templateString="${defined($$b)}", data={'b':''})['output']
 * '1'
 */
Result_t query_defined(RunCtxPtr_t ctx, GetArg_t get_arg) {
    --ctx->log_suppressed;
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
    throw std::runtime_error(__PRETTY_FUNCTION__);
}

/** Returns true if object exists.
 */
Result_t query_exists(EvalCtx_t *ctx, GetArg_t get_arg) {
    --ctx->log_suppressed;
    auto arg = get_arg();
    return ctx->frames_ptr->exists(arg);
}

/** Returns true if frag or fraglist is empty.
 */
Result_t query_isempty(RunCtxPtr_t ctx, GetArg_t get_arg) {
    --ctx->log_suppressed;
    auto arg = get_arg();
    switch (arg.type()) {
    case Value_t::tag::string:
    case Value_t::tag::string_ref:
    case Value_t::tag::integral:
    case Value_t::tag::real:
    case Value_t::tag::regex:
    case Value_t::tag::undefined:
        warn(
            ctx,
            "The path expression references object of '" + arg.type_str() + "' "
            "type with value '" + arg.printable() + "' for which isempty() "
            "query is undefined"
        );
        return Result_t();
    case Value_t::tag::frag_ref:
        return Result_t(arg.as_frag_ref().ptr->empty());
    case Value_t::tag::list_ref:
        return Result_t(arg.as_list_ref().ptr->empty());
    }
    throw std::runtime_error(__PRETTY_FUNCTION__);
}

} // namespace exec
} // namespace Teng

#endif /* TENGPROCESSORFRAG_H */

