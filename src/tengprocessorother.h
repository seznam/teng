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
Result_t dict(RunCtxPtr_t ctx, GetArg_t get_arg) {
    Value_t arg = get_arg();

    // dict member?
    if (auto *item = ctx->dict.lookup(arg.string()))
        return Result_t(*item);

    // config member?
    if (auto *item = ctx->cfg.lookup(arg.string()))
        return Result_t(*item);

    logError(*ctx, "Dictionary item '" + arg.string() + "' was not found");
    return arg;
}

/** Pushes value on program stack.
 */
void prg_stack_push(std::vector<Value_t> &prg_stack, GetArg_t get_arg) {
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
    auto &instr = ctx->instr->as<Stack_t>();
    if (instr.index > 0)
        throw std::runtime_error("Program stack underflow");
    if (-instr.index > prg_stack.size())
        throw std::runtime_error("Program stack underflow");
    // TODO(burlog): fakt je to ted dobre, kdyz je index unsigned, nemelo by se to premistit do kompilace
    return prg_stack[prg_stack.size() - 1 + instr.index];
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
        logError(*ctx, "Right operand of repeat string operator is not int");
        return Result_t();
    } else if (rhs.as_int() < 0) {
        logError(*ctx, "Right operand of repeat string operator is negative");
        return Result_t();
    } else if (!lhs.is_string_like()) {
        logError(*ctx, "Left operand of repeat string operator is not string");
        return Result_t();
    }

    // exec operator
    std::string result;
    result.reserve(lhs.string().size() * rhs.as_int());
    for (auto i = 0; i < rhs.as_int(); ++i)
        result += lhs.string();
    return Result_t(std::move(result));
}

/** Evaluates function if such exists.
 */
Result_t func(EvalCtx_t *ctx, GetArg_t get_arg) {
    auto &instr = ctx->instr->template as<Func_t>();

    // make function context object
    auto fun_ctx = FunctionCtx_t(
        ctx->err,
        ctx->encoding,
        ctx->escaper_ptr,
        ctx->cfg,
        ctx->dict
    );

    // prepare args
    FunctionArgs_t args;
    for (auto i = instr.nargs; i > 0; --i)
        args.push_back(get_arg());

    if (!instr.is_udf) {
        // builtin functions
        if (auto function = findFunction(instr.name))
            return function(ctx, fun_ctx, args);

    } else {
        // user defined functions
        if (auto function = udf::findFunction(instr.name))
            return function(ctx, fun_ctx, args);
    }

    // no such function
    logError(
        *ctx,
        "call of unknown function " + instr.name + "()"
    );
    return Result_t(1);
}

/** Writes string value of top item on stack (arg) to output.
 */
void print(RunCtxPtr_t ctx, GetArg_t get_arg) {
    get_arg().print([&] (const string_view_t &v, auto &&tag) {
        if (Value_t::visited_value(tag) == Value_t::tag::frag_ref)
            logWarning(*ctx, "Variable is a fragment, not a scalar value");
        if (Value_t::visited_value(tag) == Value_t::tag::list_ref)
            logWarning(*ctx, "Variable is a fragment list, not a scalar value");
        if (Value_t::visited_value(tag) == Value_t::tag::regex)
            logWarning(*ctx, "Variable is a regex, not a scalar value");
        ctx->output.write(v);
    });
}

/** Push new formatter on formatter stack.
 */
void push_formatter(RunCtxPtr_t ctx) {
    if (ctx->cfg.isFormatEnabled()) {
        auto &instr = ctx->instr->as<Format_t>();
        auto mode = static_cast<Formatter_t::Mode_t>(instr.mode);
        if (mode == Formatter_t::MODE_COPY_PREV)
            mode = ctx->output.top();
        ctx->output.push(mode);
    }
}

/** Pop current formatter from formatter stack.
 */
void pop_formatter(RunCtxPtr_t ctx) {
    if (ctx->cfg.isFormatEnabled())
        if (ctx->output.pop() == Formatter_t::MODE_INVALID)
            throw std::runtime_error("stack of formatters is corrupted");
}

/** Push new escaper on the escapers stak.
 */
void push_escaper(RunCtxPtr_t ctx) {
    auto &instr = ctx->instr->as<CType_t>();
    ctx->escaper.push(instr.ctype? instr.ctype->contentType.get(): nullptr);
}

/** Pop current escaper from escapers stack.
 */
void pop_escaper(RunCtxPtr_t ctx) {
    if (ctx->escaper.size() == 1) {
        logError(*ctx, "Can't pop content type: only one remains.");
    } else ctx->escaper.pop();
}

/** Evaluates matching of regular expression.
 */
Result_t regex_match(EvalCtx_t *ctx, GetArg_t get_arg) {
    auto &instr = ctx->instr->template as<RegexMatch_t>();
    return Result_t(instr.matches(get_arg().printable()));
}


} // namespace exec
} // namespace Teng

#endif /* TENGPROCESSOROTHER_H */

