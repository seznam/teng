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

#include <tengfp.h>
#include <tengprocessorcontext.h>

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
std::string to_string(std::greater_equal<>) {return ">=";}
std::string to_string(std::greater<>) {return ">";}

/** Almost all operations accept any lhs value.
 */
template <typename operation_t, typename = void>
struct lhs_checker_t {
    static bool is_valid(EvalCtx_t *, Value_t &) {return true;}
};

/** Almost all operations accept any lhs value.
 */
template <typename operation_t, typename = void>
struct rhs_checker_t {
    static bool is_valid(EvalCtx_t *, Value_t &) {return true;}
};

/** Shortcut that become valid expression if operation_t is one of bit_or,
 * bit_and or bit_xor.
 */
template <typename operation_t>
using is_bit_op = typename std::enable_if<
    std::is_same<operation_t, bit_or_t>::value
    || std::is_same<operation_t, bit_and_t>::value
    || std::is_same<operation_t, bit_xor_t>::value
>::type;

/** Bit OR needs integral operands.
 */
template <typename operation_t>
struct lhs_checker_t<operation_t, is_bit_op<operation_t>> {
    static bool is_valid(EvalCtx_t *ctx, Value_t &lhs) {
        if (lhs.is_integral()) return true;
        logError(
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
struct rhs_checker_t<operation_t, is_bit_op<operation_t>> {
    static bool is_valid(EvalCtx_t *ctx, Value_t &rhs) {
        if (rhs.is_integral()) return true;
        logError(
            *ctx,
            "The right operand of " + to_string(operation_t())
            + " numeric operator is a " + rhs.type_str()
            + " but an integer is expected"
        );
        return false;
    }
};

/** Shortcut that become valid expression if operation_t is some kind of
 * division.
 */
template <typename operation_t>
using needs_rhs_non_zero = typename std::enable_if<
    std::is_same<operation_t, std::divides<>>::value
    || std::is_same<operation_t, modulus_t>::value
>::type;

/** Right operand of divison operations must not be zero.
 */
template <typename operation_t>
struct rhs_checker_t<operation_t, needs_rhs_non_zero<operation_t>> {
    static bool is_valid(EvalCtx_t *ctx, Value_t &rhs) {
        bool is_modulus = std::is_same<operation_t, modulus_t>::value;
        static const std::string S = to_string(operation_t());
        if (is_modulus && rhs.integral()) return true;
        else if (!is_modulus && rhs.real()) return true;
        logError(*ctx, "Right operand of " + S + " division operator is zero");
        return false;
    }
};

} // namespace

/** Evaluates binary numeric operation.
 */
template <typename get_arg_t, typename operation_t>
Result_t numop(EvalCtx_t *ctx, get_arg_t get_arg, operation_t operation) {
    static const std::string S = to_string(operation_t());

    // fetch operator args
    // remember, they are on stack so get them in opposite order
    Value_t rhs = get_arg();
    Value_t lhs = get_arg();

    // report invalid operands
    if (lhs.is_string() || lhs.is_undefined()) {
        auto tp = lhs.type_str();
        logError(*ctx, "Left operand of " + S + " numeric operator is " + tp);
        return Result_t();
    }
    if (rhs.is_string() || rhs.is_undefined()) {
        auto tp = rhs.type_str();
        logError(*ctx, "Right operand of " + S + " numeric operator is " + tp);
        return Result_t();
    }

    // some operations have restrictions for operands
    if (!lhs_checker_t<operation_t>::is_valid(ctx, lhs))
        return Result_t();
    if (!rhs_checker_t<operation_t>::is_valid(ctx, rhs))
        return Result_t();

    // prepare error callback for floating point calculation
    auto fp_error = [&] {
        logError(*ctx, "Floating point operation failed");
        return Result_t();
    };

    // prepare floating point callback
    auto fp_operation = [&] {
        return Result_t(operation(lhs.real(), rhs.real()));
    };

    // mod make sense only for integral operands
    bool is_modulus = std::is_same<operation_t, modulus_t>::value;

    // exec operation
    return !is_modulus && (lhs.is_real() || rhs.is_real())
        ? fp_safe(fp_operation, fp_error)
        : Result_t(operation(lhs.integral(), rhs.integral()));
}

/** Evaluates binary string operation.
 */
template <typename get_arg_t, typename operation_t>
Result_t strop(EvalCtx_t *ctx, get_arg_t get_arg, operation_t operation) {
    // fetch operator args
    // remember, they are on stack so get them in opposite order
    Value_t rhs = get_arg();
    Value_t lhs = get_arg();

    // some operations have restrictions for operands
    if (!lhs_checker_t<operation_t>::is_valid(ctx, lhs))
        return Result_t();
    if (!rhs_checker_t<operation_t>::is_valid(ctx, rhs))
        return Result_t();

    // exec operation
    return Result_t(operation(lhs.str(), rhs.str()));
}

/** Implementation of the logic not operator.
 */
template <typename get_arg_t>
Result_t logic_not(EvalCtx_t *ctx, get_arg_t get_arg) {
    auto arg = get_arg();
    return arg.is_undefined()? arg: Result_t(!arg);
}

/** Implementation of the bit not operator.
 */
template <typename get_arg_t>
Result_t bit_not(EvalCtx_t *ctx, get_arg_t get_arg) {
    Value_t arg = get_arg();
    if (arg.is_integral())
        return Result_t(~arg.as_int());
    logError(*ctx, "operand of bit ~ operation is not int");
    return Result_t();
}

} // namespace exec
} // namespace Teng

#endif /* TENGPROCESSOROPS_H */

