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
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz
 *
 * HISTORY
 * 2018-07-07  (burlog)
 *             Extracted from tengprocessor.cc.
 */

#ifndef TENGPROCESSOROPS_H
#define TENGPROCESSOROPS_H

#include "fp.h"
#include "processorcontext.h"

namespace Teng {
namespace exec {
namespace {

// shortcuts
using bit_or_t = std::bit_or<int64_t>;
using bit_xor_t = std::bit_xor<int64_t>;
using bit_and_t = std::bit_and<int64_t>;
using modulus_t = std::modulus<int64_t>;

// These functions are used to descibe operations in log messages.
std::string to_string(bit_or_t) {return "|";}
std::string to_string(bit_xor_t) {return "^";}
std::string to_string(bit_and_t) {return "&";}
std::string to_string(std::plus<>) {return "+";}
std::string to_string(std::minus<>) {return "-";}
std::string to_string(std::multiplies<>) {return "*";}
std::string to_string(std::divides<>) {return "/";}
std::string to_string(modulus_t) {return "%";}
std::string to_string(std::equal_to<>) {return "==";}
std::string to_string(std::not_equal_to<>) {return "!=";}
std::string to_string(std::greater_equal<>) {return ">=";}
std::string to_string(std::greater<>) {return ">";}
std::string to_string(std::less_equal<>) {return "<=";}
std::string to_string(std::less<>) {return "<";}

/** Shortcut for enable_if_t.
 */
template <typename type_t, template <typename> class pred_t>
using when = std::enable_if_t<pred_t<type_t>::value, bool>;

/** Shortcut for enable_if_t.
 */
template <typename type_t, template <typename> class pred_t>
using unless = std::enable_if_t<!pred_t<type_t>::value, bool>;

/** The compile time constant that become true if operation is one of bit_or,
 * bit_and or bit_xor.
 */
template <typename operation_t>
using is_bit_op = std::integral_constant<
    bool,
    std::is_same<std::decay_t<operation_t>, bit_or_t>::value
    || std::is_same<std::decay_t<operation_t>, bit_and_t>::value
    || std::is_same<std::decay_t<operation_t>, bit_xor_t>::value
>;

/** The compile time constant that become true if operation is some kind of
 * division.
 */
template <typename operation_t>
using is_division = std::integral_constant<
    bool,
    std::is_same<operation_t, std::divides<>>::value
    || std::is_same<operation_t, modulus_t>::value
>;

/** The compile time constant that become true if operation needs integral
 * argument.
 */
template <typename operation_t>
using is_integral_op = std::integral_constant<
    bool,
    is_bit_op<operation_t>::value
    || std::is_same<operation_t, modulus_t>::value
>;

/** Almost all operations accept any lhs value.
 */
template <typename operation_t, typename = bool>
struct lhs_checker_t {
    static bool is_valid(EvalCtx_t *, Value_t &) {return true;}
};

/** Almost all operations accept any rhs value.
 */
template <typename operation_t, typename = bool>
struct rhs_checker_t {
    static bool is_valid(EvalCtx_t *, Value_t &) {return true;}
};

/** Bit operations needs integral operands.
 */
template <typename operation_t>
struct lhs_checker_t<operation_t, when<operation_t, is_bit_op>> {
    static bool is_valid(EvalCtx_t *ctx, Value_t &lhs) {
        if (lhs.is_integral()) return true;
        logWarning(
            *ctx,
            "The left operand of " + to_string(operation_t())
            + " numeric operator is a " + lhs.type_str()
            + " but an integer is expected"
        );
        return false;
    }
};

/** Bit OR needs integral operands.
 */
template <typename operation_t>
struct rhs_checker_t<operation_t, when<operation_t, is_bit_op>> {
    static bool is_valid(EvalCtx_t *ctx, Value_t &rhs) {
        if (rhs.is_integral()) return true;
        logWarning(
            *ctx,
            "The right operand of " + to_string(operation_t())
            + " numeric operator is a " + rhs.type_str()
            + " but an integer is expected"
        );
        return false;
    }
};

/** Right operand of divison operations must not be zero.
 */
template <typename operation_t>
struct rhs_checker_t<operation_t, when<operation_t, is_division>> {
    static bool is_valid(EvalCtx_t *ctx, Value_t &rhs) {
        bool is_modulus = std::is_same<operation_t, modulus_t>::value;
        static const std::string S = to_string(operation_t());
        if (is_modulus && rhs.integral()) return true;
        else if (!is_modulus && rhs.real()) return true;
        logWarning(
            *ctx,
            "Right operand of " + S + " division operator is zero"
        );
        return false;
    }
};

} // namespace

/** Evaluates binary numeric operation: implementation for all operation.
 */
template <typename operation_t, unless<operation_t, is_integral_op> = false>
Result_t exec_op(EvalCtx_t *ctx, Value_t &lhs, Value_t &rhs, operation_t op) {
    // prepare error callback for floating point calculation
    auto fp_error = [&] {
        logWarning(*ctx, "Floating point operation failed");
        return Result_t();
    };

    // prepare floating point callback
    auto fp_operation = [&] {
        return Result_t(op(lhs.real(), rhs.real()));
    };

    // mod make sense only for integral operands
    bool is_modulus = std::is_same<operation_t, modulus_t>::value;

    // exec operation
    return !is_modulus && (lhs.is_real() || rhs.is_real())
        ? fp_safe(fp_operation, fp_error)
        : Result_t(op(lhs.integral(), rhs.integral()));
}

/** Shouldn't be called: implementation for bit operation.
 */
template <typename operation_t, when<operation_t, is_integral_op> = true>
Result_t exec_op(EvalCtx_t *, Value_t &lhs, Value_t &rhs, operation_t op) {
    return Result_t(op(lhs.integral(), rhs.integral()));
}

/** Evaluates binary numeric operation.
 */
template <typename operation_t>
Result_t numop(EvalCtx_t *ctx, Value_t &lhs, Value_t &rhs, operation_t op) {
    static const std::string S = to_string(operation_t());

    // report invalid operands
    if (!lhs.is_number()) {
        logWarning(
            *ctx,
            "Left operand of " + S + " numeric operator is " + lhs.type_str()
        );
        return Result_t();
    }
    if (!rhs.is_number()) {
        logWarning(
            *ctx,
            "Right operand of " + S + " numeric operator is " + rhs.type_str()
        );
        return Result_t();
    }

    // some operations have restrictions for operands
    if (!lhs_checker_t<operation_t>::is_valid(ctx, lhs))
        return Result_t();
    if (!rhs_checker_t<operation_t>::is_valid(ctx, rhs))
        return Result_t();

    // execute op
    return exec_op(ctx, lhs, rhs, op);
}

/** Evaluates binary string operation.
 */
template <typename operation_t>
Result_t strop(EvalCtx_t *ctx, Value_t &lhs, Value_t &rhs, operation_t op) {
    // some operations have restrictions for operands
    if (!lhs_checker_t<operation_t>::is_valid(ctx, lhs))
        return Result_t();
    if (!rhs_checker_t<operation_t>::is_valid(ctx, rhs))
        return Result_t();

    // exec operation
    return Result_t(op(lhs.ensure_string_like(), rhs.ensure_string_like()));
}

/** Evaluates binary numeric operation.
 */
template <typename operation_t>
Result_t numop(EvalCtx_t *ctx, GetArg_t get_arg, operation_t op) {
    // fetch operator args
    // remember, they are on stack so get them in opposite order
    Value_t rhs = get_arg();
    Value_t lhs = get_arg();

    // evalute
    return numop(ctx, lhs, rhs, op);
}

/** Evaluates binary string operation.
 */
template <typename operation_t>
Result_t strop(EvalCtx_t *ctx, GetArg_t get_arg, operation_t op) {
    // fetch operator args
    // remember, they are on stack so get them in opposite order
    Value_t rhs = get_arg();
    Value_t lhs = get_arg();

    // evaluate
    return strop(ctx, lhs, rhs, op);
}

/** Evaluates binary string or numeric operation.
 */
template <typename operation_t>
Result_t strnumop(EvalCtx_t *ctx, GetArg_t get_arg, operation_t op) {
    // fetch operator args
    // remember, they are on stack so get them in opposite order
    Value_t rhs = get_arg();
    Value_t lhs = get_arg();

    // if at least one operand is string use string version of operator
    return lhs.is_string_like() || rhs.is_string_like()
        ? strop(ctx, lhs, rhs, op)
        : numop(ctx, lhs, rhs, op);
}

/** Implementation of the logic not operator.
 */
Result_t logic_not(EvalCtx_t *, GetArg_t get_arg) {
    auto arg = get_arg();
    return arg.is_undefined()? arg: Result_t(!arg);
}

/** Implementation of the bit not operator.
 */
Result_t bit_not(EvalCtx_t *ctx, GetArg_t get_arg) {
    Value_t arg = get_arg();
    if (arg.is_integral())
        return Result_t(~arg.as_int());
    logWarning(*ctx, "Operand of bit ~ operator is not int");
    return Result_t();
}

/** Implementation of the unary plus operator.
 */
Result_t unary_plus(EvalCtx_t *ctx, GetArg_t get_arg) {
    Value_t arg = get_arg();
    switch (arg.type()) {
    case Value_t::tag::integral:
        return Result_t(arg.as_int());
    case Value_t::tag::real:
        return Result_t(arg.as_real());
    default:
        logWarning(*ctx, "Operand of unary + operator is not number");
        return Result_t();
    }
}

/** Implementation of the unary minus operator.
 */
Result_t unary_minus(EvalCtx_t *ctx, GetArg_t get_arg) {
    Value_t arg = get_arg();
    switch (arg.type()) {
    case Value_t::tag::integral:
        return Result_t(-arg.as_int());
    case Value_t::tag::real:
        return Result_t(-arg.as_real());
    default:
        logWarning(*ctx, "Operand of unary - operator is not number");
        return Result_t();
    }
}

/** Implementation of the repeat string operator.
 */
Result_t repeat_string(EvalCtx_t *ctx, GetArg_t get_arg) {
    // fetch operator args
    // remember, they are on stack so get them in opposite order
    Value_t rhs = get_arg();
    Value_t lhs = get_arg();

    // check args
    if (!rhs.is_integral()) {
        logWarning(
            *ctx,
            "Right operand of repeat string operator is not int"
        );
        return Result_t();

    } else if (rhs.as_int() < 0) {
        logWarning(
            *ctx,
            "Right operand of repeat string operator is negative"
        );
        return Result_t();

    } else if (!lhs.is_string_like()) {
        logWarning(
            *ctx,
            "Left operand of repeat string operator is not string"
        );
        return Result_t();
    }

    // exec operator
    std::string result;
    result.reserve(lhs.string().size() * rhs.as_int());
    for (auto i = 0; i < rhs.as_int(); ++i)
        result += lhs.string();
    return Result_t(std::move(result));
}

} // namespace exec
} // namespace Teng

#endif /* TENGPROCESSOROPS_H */

