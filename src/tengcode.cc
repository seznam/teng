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
 * $Id: tengcode.cc,v 1.2 2007-12-14 08:54:16 vasek Exp $
 *
 * DESCRIPTION
 * Teng byte-code generation.
 *
 * AUTHORS
 * Stepan Skrob <stepan@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-30  (solamyl)
 *             Created.
 */

#include "tengerror.h"
#include "tengprogram.h"
#include "tenginstruction.h"
#include "tengparservalue.h"
#include "tenglogging.h"
#include "tengcode.h"

namespace Teng {
namespace {

template <typename type_t, typename opt_type_t>
std::size_t
genCode(
    Parser::Context_t *ctx,
    Instruction_t::OpCode_t opcode,
    type_t &&value,
    opt_type_t &&opt_value
) {
    // save program size/address
    auto prgsize = ctx->program->size();

    // create instruction
    ctx->program->emplace_back(
        opcode,
        Parser::Value_t(std::forward<type_t>(value)),
        Parser::Value_t(std::forward<opt_type_t>(opt_value)),
        ctx->position()
    );

    // return origin program size/address
    return prgsize;
}

} // namespace

std::size_t
generateCode(Parser::Context_t *ctx,
             Instruction_t::OpCode_t opcode,
             const Parser::Symbol_t &value,
             const Parser::Symbol_t &opt_value)
{
    return genCode(ctx, opcode, value.str(), opt_value.str());
}

unsigned int
generateExpression(Parser::Context_t *ctx,
                   unsigned int start,
                   Instruction_t::OpCode_t code1,
                   bool negated)
{
    generateCode(ctx, code1);
    if (negated) generateCode(ctx, Instruction_t::NOT);
    optimizeExpression(ctx, start);
    return start;
}

void replaceCode(Parser::Context_t *ctx,
                 Instruction_t::OpCode_t code,
                 const Parser::Symbol_t &symbol)
{
    ctx->program->clear();
    syntaxError(ctx, symbol, "Fatal parse error in template");
    generateCode(ctx, code);
}

void generateFunctionCall(Parser::Context_t *ctx,
                          const std::string &name,
                          int nparams)
{
    // be optimal for unescape($variable)
    // if last instr. is VAR and should be escaped
    // unescaping a single variable -- change escaping status of that variable
    if ((name == "unescape") && (nparams == 1)) {
        if (ctx->program->back().opcode == Instruction_t::VAR) {
            if (ctx->program->back().value) {   // means do escape
                ctx->program->back().value = 0; // set don't escape
                return;
            }
        }
    }

    // other function call -- generate code for it
    genCode(ctx, Instruction_t::FUNC, name, nparams);
}

void generatePrint(Parser::Context_t *ctx) {
    return (void)generateCode(ctx, Instruction_t::PRINT);

    // // TODO(burlog): lowestValPrintAddress nahrada
    // // get actual program size
    // auto prgsize = ctx->program->size();
    //
    // // overflow protect -> no optimalization can be peformed for now
    // if ((prgsize <= 3) || (ctx->lowestValPrintAddress + 3) >= prgsize)
    //     return (void)generateCode(ctx, Instruction_t::PRINT);
    //
    // // attempt to optimize consecutive print instrs to one merged
    // if ((*ctx->program)[prgsize - 1].opcode != Instruction_t::VAL)
    //     return (void)generateCode(ctx, Instruction_t::PRINT);
    // if ((*ctx->program)[prgsize - 2].opcode != Instruction_t::PRINT)
    //     return (void)generateCode(ctx, Instruction_t::PRINT);
    // if ((*ctx->program)[prgsize - 3].opcode != Instruction_t::VAL)
    //     return (void)generateCode(ctx, Instruction_t::PRINT);
    //
    // // optimalize sequence of VAL, PRINT, VAL, PRINT to single VAL, PRINT pair
    // (*ctx->program)[prgsize - 3].value = std::string(
    //     (*ctx->program)[prgsize - 3].value.str()
    //     + (*ctx->program)[prgsize - 1].value.str()
    // );
    //
    // // delete last VAL instruction (optimized out)
    // ctx->program->pop_back();
}

void generatePrint(Parser::Context_t *ctx, const Parser::Symbol_t &symbol) {
    generateCode(ctx, Instruction_t::VAL, symbol);
    generatePrint(ctx);
}

void optimizeExpression(Parser::Context_t *ctx, unsigned int start) {
    // try to evaluate given part of prog
    Parser::Value_t val;
    std::cerr << "optimizing: <" << start << "," << ctx->program->size() << ">" << std::endl;
    // int rc = ctx->coproc.eval(val, start, ctx->program->size());

    // // subst code on success
    // if (rc == 0) {
    //     // remove expression's program
    //     ctx->program->erase(ctx->program->begin() + start, ctx->program->end());
    //     generateCode(ctx, Instruction_t::VAL, val);
    // }
}

void eraseCodeFrom(Parser::Context_t *ctx, unsigned int start) {
    auto *program = ctx->program.get();
    program->erase(program->begin() + start, program->end());
}

void generateHalt(Parser::Context_t *ctx) {
    ctx->program->emplace_back(Instruction_t::HALT, Pos_t());
}

} // namespace Teng

