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

#ifndef TENGPROCESSOROTHER_H
#define TENGPROCESSOROTHER_H

#include <tengudf.h>
#include <tengfunction.h>
#include <tengprocessorcontext.h>

namespace Teng {
namespace exec {

/** Implementation of the teng 'defined' operator.
 */
Result_t defined(RunCtxPtr_t ctx) {
    logWarning(*ctx, "The defined() operator is deprecated");

    // no such variable and no such fragment found
    auto error_code = ctx->frag_stack.exists(ctx->instr->identifier);
    if (error_code) return Result_t(false);

    // return false if fragment
    Result_t tmp;
    error_code = ctx->frag_stack.findVariable(ctx->instr->identifier, tmp);
    if (error_code) return Result_t(true);

    // ok
    return tmp;
}

/** Implementation of the teng 'isempty' operator.
 */
Result_t isempty(RunCtxPtr_t ctx) {
    // TODO(burlog): make it
    throw std::runtime_error("----------!");
}

/** Implementation of the teng 'exists' operator.
 */
Result_t exists(RunCtxPtr_t ctx) {
    auto error_code = ctx->frag_stack.exists(ctx->instr->identifier);
    return Result_t(!error_code);
}

/** Writes debug fragment into template if it is enabled.
 */
void debuging(RunCtxPtr_t ctx) {
    throw std::runtime_error("not implemented yet");
    // if (configuration.isDebugEnabled())
    //     instructionDebug(data, output);
}

/** Writes bytecode fragment into template if it is enabled.
 */
void bytecode(RunCtxPtr_t ctx) {
    throw std::runtime_error("not implemented yet");
    // if (configuration.isBytecodeEnabled())
    //     dumpBytecode(escaper, program, output);
}

/** Implementation of the dictionary lookup function.
 */
template <typename get_arg_t>
Result_t dict(RunCtxPtr_t ctx, get_arg_t get_arg) {
    Value_t arg = get_arg();

    // dict member?
    if (auto *item = ctx->dict.lookup(arg.str()))
        return Result_t(*item);

    // config member?
    if (auto *item = ctx->cfg.lookup(arg.str()))
        return Result_t(*item);

    logError(*ctx, "Dictionary item '" + arg.str() + "' was not found");
    return arg;
}

/** Implementation of the variable lookup.
 */
Result_t var(RunCtxPtr_t ctx, bool escape) {
    Value_t var;

    // if variable does not exist then return empty string
    if (ctx->frag_stack.findVariable(ctx->instr->identifier, var)) {
        auto path = ctx->frag_stack.currentPath()
                  + "." + ctx->instr->identifier.name;
        logWarning(*ctx, "Variable '" + path + "' is undefined");
        return Result_t();
    }

    // check whether we have to escape variable
    if (ctx->cfg.isAlwaysEscapeEnabled()) {
        if (ctx->instr->value.integral() && var.is_string())
            return Result_t(ctx->escaper.escape(var.as_str()));

    } else if (escape && ctx->instr->value.is_string())
        return Result_t(ctx->escaper.escape(var.as_str()));

    // no escaping
    return var;
}

/** Pushes value on program stack.
 */
template <typename get_arg_t>
void prg_stack_push(std::vector<Value_t> &prg_stack, get_arg_t get_arg) {
    prg_stack.push_back(get_arg());
}

/** Popes value from program stack.
 */
void prg_stack_pop(std::vector<Value_t> &prg_stack) {
    move_back(prg_stack);
}

/** Returns value at instr.value index on program stack.
 */
Result_t prg_stack_at(EvalCtx_t *ctx, std::vector<Value_t> &prg_stack) {
    if (ctx->instr->value.integral() > 0)
        throw std::runtime_error("Program stack underflow");
    if (-ctx->instr->value.integral() > prg_stack.size())
        throw std::runtime_error("Program stack underflow");
    return prg_stack[prg_stack.size() - 1 + ctx->instr->value.integral()];
}

/** Implementation of the repeat string operator.
 */
template <typename get_arg_t>
Result_t repeat_string(EvalCtx_t *ctx, get_arg_t get_arg) {
    // fetch operator args
    // remember, they are on stack so get them in opposite order
    Value_t rhs = get_arg();
    Value_t lhs = get_arg();

    // check args
    if (!rhs.is_integral()) {
        logError(*ctx, "Right operand of repeat string operator is not int");
        return Result_t();
    } else if (rhs.as_int() < 0) {
        logError(*ctx, "Right operand of repeat string operator is negative");
        return Result_t();
    } else if (!lhs.is_string()) {
        logError(*ctx, "Left operand of repeat string operator is not string");
        return Result_t();
    }

    // exec operator
    std::string result;
    result.reserve(lhs.as_str().size() * rhs.as_int());
    for (auto i = 0; i < rhs.as_int(); ++i)
        result += lhs.as_str();
    return Result_t(std::move(result));
}

/** Evaluates function if such exists.
 */
template <typename get_arg_t>
Result_t func(RunCtxPtr_t ctx, get_arg_t get_arg) {
    if (ctx->instr->value.integral() < 0)
        throw std::runtime_error("negative function args count");

    // make function context object
    auto fun_ctx = FunctionCtx_t(
        ctx->err,
        ctx->encoding,
        ctx->escaper, // ???
        ctx->cfg,
        ctx->dict,
        &ctx->frag_stack // ???
    );

    // prepare args
    FunctionArgs_t args;
    for (auto i = ctx->instr->opt_value.integral(); i > 0; --i)
        args.push_back(get_arg());

    // builtin functions
    if (auto function = findFunction(ctx->instr->value.str()))
        return function(ctx, fun_ctx, args);

    // user defined functions
    if (auto function = udf::findFunction(ctx->instr->value.str()))
        return function(ctx, fun_ctx, args);

    // no such function
    logError(
        *ctx,
        "call of unknown function " + ctx->instr->value.str() + "()"
    );
    return Result_t();
}

/** Writes string value of top item on stack (arg) to output.
 */
template <typename get_arg_t>
void print(RunCtxPtr_t ctx, get_arg_t get_arg) {
    ctx->output.write(get_arg().str());
}

/** Push new formatter on formatter stack.
 */
void push_formatter(RunCtxPtr_t ctx) {
    if (ctx->cfg.isFormatEnabled()) {
        auto mode = ctx->instr->value.integral();
        ctx->output.push(static_cast<Formatter_t::Mode_t>(mode));
    }
}

/** Pop current formatter from formatter stack.
 */
void pop_formatter(RunCtxPtr_t ctx) {
    if (ctx->cfg.isFormatEnabled())
        if (ctx->output.pop() == Formatter_t::MODE_INVALID)
            throw std::runtime_error("stack of formatters is corrupted");
}

/** Set variable value.
 */
template <typename get_arg_t>
void set_var(RunCtxPtr_t ctx, get_arg_t get_arg) {
    Value_t arg = get_arg();
    switch (ctx->frag_stack.setVariable(ctx->instr->identifier, arg)) {
    case S_OK:
        break;
    case S_ALREADY_DEFINED:
        logWarning(
            *ctx,
            "Cannot rewrite variable '" + ctx->instr->identifier.name
            + "' which is already set by the application"
        );
        break;
    default:
        logWarning(
            *ctx,
            "Cannot set variable '" + ctx->instr->identifier.name + "'"
        );
        break;
    }
}

/** Push new escaper on the escapers stak.
 */
void push_escaper(RunCtxPtr_t ctx) {
    auto id = ctx->instr->value.integral();
    ctx->escaper.push(id, ctx->err, position(ctx->instr));
}

/** Pop current escaper from escapers stack.
 */
void pop_escaper(RunCtxPtr_t ctx) {
    ctx->escaper.pop(ctx->err, position(ctx->instr));
}

} // namespace exec
} // namespace Teng

#endif /* TENGPROCESSOROTHER_H */

