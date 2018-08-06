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

#include <tengprocessorcontext.h>

namespace Teng {
namespace exec {

/** Push frag on stack.
 */
int open_frag(RunCtxPtr_t ctx) {
    // push fragment list on stack if it exist
    auto error_code = ctx->frag_stack.pushFrame(ctx->instr->identifier);
    if (!error_code) return 0;

    // fragment has no iterations => ok, jump over fragment
    return ctx->instr->value.integral();
}

/** Pop frag from top of the stack.
 */
int close_frag(RunCtxPtr_t ctx) {
    // move from current frag to next one
    auto succes_code = ctx->frag_stack.nextIteration();
    if (succes_code) return ctx->instr->value.integral();

    // pop list of fragments from stack
    auto error_code = ctx->frag_stack.popFrame();
    if (error_code) throw std::runtime_error("fragment stack underflow");

    // don't move instruction pointer
    return 0;
}

/** Returns the number of fragments in current opened fragment list.
 */
Result_t frag_count(RunCtxPtr_t ctx) {
    uint32_t size = 0;
    if (ctx->frag_stack.getFragmentSize(ctx->instr->identifier, size)) {
        logWarning(
            *ctx,
            "fragment '" + ctx->instr->identifier.name
            + "' doesn't exist, cannot determine its size"
        );
    }
    return Result_t(size);
}

/** Returns the number of fragments in unopened (nested) fragment list.
 */
Result_t nested_frag_count(RunCtxPtr_t ctx) {
    uint32_t size = 0;
    if (ctx->frag_stack.getSubFragmentSize(ctx->instr->identifier, size)) {
        logWarning(
            *ctx,
            "fragment '" + ctx->instr->identifier.name
            + "' doesn't exist, cannot determine its size"
        );
    }
    return Result_t(size);
}

/** Returns index of current fragmnet in opened fragment list.
 */
Result_t frag_index(RunCtxPtr_t ctx, uint32_t *size = nullptr) {
    uint32_t index = 0;
    if (ctx->frag_stack.getFragmentIndex(ctx->instr->identifier, index, size)) {
        logWarning(
            *ctx,
            "fragment '" + ctx->instr->identifier.name
            + "' doesn't exist, cannot determine current fragment index"
        );
    }
    return Result_t(index);
}

/** Returns true if current fragment is the first in opened fragment list.
 */
Result_t first_frag(RunCtxPtr_t ctx) {
    return Result_t(frag_index(ctx).as_int() == 0);
}

/** Returns true if current fragment is the last in opened fragment list.
 */
Result_t last_frag(RunCtxPtr_t ctx) {
    uint32_t size = 0;
    auto index = frag_index(ctx, &size).as_int();
    return Result_t(index == (size - 1));
}

/** Returns true if the current fragment is not the first neither the last.
 */
Result_t inner_frag(RunCtxPtr_t ctx) {
    uint32_t size = 0;
    auto index = frag_index(ctx, &size).as_int();
    return Result_t((index > 0) && (index < (size - 1)));
}

/** 
 */
template <typename get_arg_t>
Result_t push_attr_at(RunCtxPtr_t ctx, get_arg_t get_arg) {
    auto i = get_arg();
    auto arg = get_arg();

    // ensure type stored in arg
    if (!arg.is_pointer()) {
        logError(*ctx, "Expected pointer to fragment or fragment value");
        return arg;
    }

    // prepare callback for fragments
    auto get_frag_attr = [&] (const Fragment_t &frag) {
        auto iattr = frag.find(i.as_str());
        if (iattr != frag.end())
            return Result_t(iattr->second.get(), FRAG_PTR::VALUE);
        logWarning(*ctx, "Unable to locate member (1) '" + i.as_str() + "'");
        return Result_t(nullptr, FRAG_PTR::NULLPTR);
    };

    // prepare callback for fragment values
    auto get_value_attr = [&] (const FragmentValue_t &value) {
        if (auto *nested = value.getNestedFragments()) {
            return nested->size() == 1
                ? get_frag_attr(*(*nested)[0])
                : Result_t(nested, FRAG_PTR::LIST);
        }

        logWarning(
            *ctx,
            "Unable to locate member (2) '" + i.as_str() + "' in fragment value"
        );
        return Result_t(nullptr, FRAG_PTR::NULLPTR);
    };

    // prepare callback for fragment lists
    auto get_list_item = [&] (const FragmentList_t &list) {
        auto idx = i.integral();
        if ((idx >= 0) && (idx < list.size()))
           return Result_t(list[idx], FRAG_PTR::FRAGMENT);
        logWarning(
            *ctx,
            "Index " + std::to_string(idx)
            + " is out of range <0," + std::to_string(list.size()) + ">"
        );
        return Result_t(nullptr, FRAG_PTR::NULLPTR);
    };

    // prepare callback for fragment values
    auto get_value_item = [&] (const FragmentValue_t &value) {
        if (auto *nested = value.getNestedFragments()) {
            auto idx = i.integral();
            if ((idx >= 0) && (idx < nested->size()))
                return Result_t((*nested)[idx], FRAG_PTR::FRAGMENT);
            logWarning(
                *ctx,
                "Index " + std::to_string(idx)
                + " is out of range <0," + std::to_string(nested->size()) + ">"
            );
            return Result_t(nullptr, FRAG_PTR::NULLPTR);
        }
        logWarning(*ctx, "Only fragment lists can be indexed");
        return Result_t(nullptr, FRAG_PTR::NULLPTR);
    };

    // process stored value
    switch (i.type()) {
    case Value_t::tag::real:
    case Value_t::tag::pointer:
    case Value_t::tag::undefined:
        logWarning(
            *ctx,
            "The value of " + i.type_str() + " type can't be used as index"
        );
        return arg;

    case Value_t::tag::string:
         switch (arg.template pointer_type<FRAG_PTR>()) {
         case FRAG_PTR::FRAGMENT:
             return get_frag_attr(*arg.template pointer<Fragment_t>());
         case FRAG_PTR::VALUE:
             return get_value_attr(*arg.template pointer<FragmentValue_t>());
         case FRAG_PTR::LIST:
         case FRAG_PTR::NULLPTR:
             logWarning(*ctx, "String indices can be used for fragments only");
             return Result_t(nullptr, FRAG_PTR::NULLPTR);
         }
         break;

    case Value_t::tag::integral:
         switch (arg.template pointer_type<FRAG_PTR>()) {
         case FRAG_PTR::LIST:
             return get_list_item(*arg.template pointer<FragmentList_t>());
         case FRAG_PTR::VALUE:
             return get_value_item(*arg.template pointer<FragmentValue_t>());
         case FRAG_PTR::FRAGMENT:
         case FRAG_PTR::NULLPTR:
             logWarning(*ctx, "Only fragment lists and values can be indexed");
             return Result_t(nullptr, FRAG_PTR::NULLPTR);
         }
         break;
    }
}

/** 
 */
Result_t push_root_frag(EvalCtx_t *ctx) {
    std::cerr << ">>>>>>>>>>>>>>>>>>> push_root_frag" << std::endl;
    //return Result_t(&ctx->root, FRAG_PTR::FRAGMENT);
    throw runtime_ctx_needed_t{};
}

/** 
 */
Result_t push_root_frag(RunCtx_t *ctx) {
#warning tag tohle je hodne spatne, root je cesta na fs, ne frag
    return Result_t(&ctx->root, FRAG_PTR::FRAGMENT);
}

/** 
 */
Result_t push_this_frag(RunCtxPtr_t ctx) {
    return Result_t(ctx->frag_stack.getCurrentFragment(), FRAG_PTR::FRAGMENT);
}

/** 
 */
template <typename get_arg_t>
Result_t push_attr(RunCtxPtr_t ctx, get_arg_t get_arg) {
    auto arg = get_arg();
    auto member = ctx->instr->value.str();

    // ensure type stored in arg
    if (!arg.is_pointer()) {
        logError(*ctx, "Expected pointer to fragment or fragment value");
        return arg;
    }

    // prepare callback for fragments
    auto get_frag_attr = [&] (const Fragment_t &frag) {
        auto iattr = frag.find(member);
        if (iattr != frag.end())
            return Result_t(iattr->second.get(), FRAG_PTR::VALUE);
        logWarning(*ctx, "Unable to locate member (3) '" + member + "'");
        return Result_t(nullptr, FRAG_PTR::NULLPTR);
    };

    // prepare callback for fragment lists
    auto get_list_attr = [&] (const FragmentList_t &) {
        logWarning(
            *ctx,
            "Unable to locate member (4) '" + member + "' in fragment list"
        );
        return Result_t(nullptr, FRAG_PTR::NULLPTR);
    };

    // prepare callback for fragment values
    auto get_value_attr = [&] (const FragmentValue_t &value) {
        if (auto *nested = value.getNestedFragments()) {
            return nested->size() == 1
                ? get_frag_attr(*(*nested)[0])
                : Result_t(nested, FRAG_PTR::LIST);
        }

        logWarning(
            *ctx,
            "Unable to locate member (5) '" + member + "' in fragment value"
        );
        return Result_t(nullptr, FRAG_PTR::NULLPTR);
    };

    // process stored value
    switch (arg.template pointer_type<FRAG_PTR>()) {
    case FRAG_PTR::NULLPTR:
        return arg;
    case FRAG_PTR::FRAGMENT:
        return get_frag_attr(*arg.template pointer<Fragment_t>());
    case FRAG_PTR::LIST:
        return get_list_attr(*arg.template pointer<FragmentList_t>());
    case FRAG_PTR::VALUE:
        return get_value_attr(*arg.template pointer<FragmentValue_t>());
    }
    throw std::runtime_error(__PRETTY_FUNCTION__);
}

/** 
 */
template <typename get_arg_t>
Result_t repr(RunCtxPtr_t ctx, get_arg_t get_arg) {
    auto arg = get_arg();

    // ensure type stored in arg
    if (!arg.is_pointer()) {
        logError(*ctx, "Expected pointer to fragment or fragment value");
        return arg;
    }

    // process value
    switch (arg.template pointer_type<FRAG_PTR>()) {
    case FRAG_PTR::NULLPTR:
        return Result_t("$null$");
    case FRAG_PTR::FRAGMENT:
        return Result_t("$frag$");
    case FRAG_PTR::LIST:
        return Result_t("$fraglist$");
    case FRAG_PTR::VALUE:
        if (auto *ptr = arg.template pointer<FragmentValue_t>()) {
            return ptr->getNestedFragments()
                ? Result_t("$fraglist$")
                : Result_t(ctx->escaper.escape(ptr->getValue()));
        }
        break;
    }
    throw std::runtime_error(__PRETTY_FUNCTION__);
}

/** 
 */
template <typename get_arg_t>
Result_t repr_jsonify(RunCtxPtr_t ctx, get_arg_t get_arg) {
    auto arg = get_arg();

    // ensure type stored in arg
    if (!arg.is_pointer()) {
        logError(*ctx, "Expected pointer to fragment or fragment value");
        return arg;
    }

    // process value
    std::stringstream os;
    switch (arg.template pointer_type<FRAG_PTR>()) {
    case FRAG_PTR::NULLPTR:
        break;
    case FRAG_PTR::FRAGMENT:
        arg.template pointer<Fragment_t>()->json(os);
        break;
    case FRAG_PTR::LIST:
        arg.template pointer<FragmentList_t>()->json(os);
        break;
    case FRAG_PTR::VALUE:
        arg.template pointer<FragmentValue_t>()->json(os);
        break;
    }
    return Result_t(os.str());
}

/** 
 */
template <typename get_arg_t>
Result_t repr_count(RunCtxPtr_t ctx, get_arg_t get_arg) {
    auto arg = get_arg();

    // ensure type stored in arg
    if (!arg.is_pointer()) {
        logError(*ctx, "Expected pointer to fragment or fragment value");
        return arg;
    }

    // process value
    switch (arg.template pointer_type<FRAG_PTR>()) {
    case FRAG_PTR::NULLPTR:
        return Result_t("null");
    case FRAG_PTR::FRAGMENT:
        return Result_t(1);
    case FRAG_PTR::LIST:
        return Result_t(arg.template pointer<FragmentList_t>()->size());
    case FRAG_PTR::VALUE:
        if (auto *ptr = arg.template pointer<FragmentValue_t>()) {
            if (auto *list = ptr->getNestedFragments())
                return Result_t(list->size());
            return Result_t(1);
        }
        break;
    }
    throw std::runtime_error(__PRETTY_FUNCTION__);
}

/** 
 */
template <typename get_arg_t>
Result_t repr_type(RunCtxPtr_t ctx, get_arg_t get_arg) {
    auto arg = get_arg();

    // ensure type stored in arg
    if (!arg.is_pointer()) {
        logError(*ctx, "Expected pointer to fragment or fragment value");
        return arg;
    }

    // process value
    switch (arg.template pointer_type<FRAG_PTR>()) {
    case FRAG_PTR::NULLPTR:
        return Result_t("null");
    case FRAG_PTR::FRAGMENT:
        return Result_t("frag");
    case FRAG_PTR::LIST:
        return Result_t("list");
    case FRAG_PTR::VALUE:
        if (auto *ptr = arg.template pointer<FragmentValue_t>()) {
            return ptr->getNestedFragments()
                ? Result_t("valuelist")
                : Result_t("value");
        }
        break;
    }
    throw std::runtime_error(__PRETTY_FUNCTION__);
}

/** 
 */
template <typename get_arg_t>
Result_t repr_defined(RunCtxPtr_t ctx, get_arg_t get_arg) {
    throw std::runtime_error("not implemented yet");
}

/** 
 */
template <typename get_arg_t>
Result_t repr_exists(RunCtxPtr_t ctx, get_arg_t get_arg) {
    --ctx->log_suppressed;
    auto arg = get_arg();

    // ensure type stored in arg
    if (!arg.is_pointer()) {
        logError(*ctx, "Expected pointer to fragment or fragment value");
        return arg;
    }

    // process value
    switch (arg.template pointer_type<FRAG_PTR>()) {
    case FRAG_PTR::NULLPTR:
        return Result_t(0);
    case FRAG_PTR::FRAGMENT:
    case FRAG_PTR::LIST:
    case FRAG_PTR::VALUE:
        return Result_t(1);
    }
    throw std::runtime_error(__PRETTY_FUNCTION__);
}

/** 
 */
template <typename get_arg_t>
Result_t repr_isempty(RunCtxPtr_t ctx, get_arg_t get_arg) {
    throw std::runtime_error("not implemented yet");
}

} // namespace exec
} // namespace Teng

#endif /* TENGPROCESSORFRAG_H */

