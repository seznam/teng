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
 * $Id: tengcode.cc,v 1.1 2004-07-28 11:36:55 solamyl Exp $
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
#include "tenginstruction.h"
#include "tengparservalue.h"
#include "tengcode.h"


namespace Teng {

/** Generate byte-code for general instruction.
  * @param context A parser context.
  * @param code Instruction code.
  * @param value Optional instruction parameter value(s). */
void tengCode_generate(ParserContext_t *context,
        Instruction_t::OpCode_t code,
        const ParserValue_t &value /*= ParserValue_t()*/)
{
    // get source index (instead of full filename)
    int srcidx = -1; //may be undefined
    if (context->sourceIndex.empty() == 0) {
        srcidx = context->sourceIndex.top();
    }
    // get position
    Error_t::Position_t pos;
    if (context->lex1.empty() == 0) {
        pos = context->lex1.top()->getPosition();
    }
    // create instruction
    context->program->push_back(Instruction_t(code, value,
            srcidx, pos.lineno, pos.col));
}


/** Generate byte-code for a function call.
  * Also optimize 'unescape($variable)' call.
  * @param context A parser context.
  * @param name The function name.
  * @param nparams Number of params in the call. */
void tengCode_generateFunctionCall(ParserContext_t *context,
        const string &name, int nparams)
{
    // be optimal for unescape($variable)
    if (name == "unescape"
            && nparams == 1
            && context->program->back().operation //if last instr. is VAR
            == Instruction_t::VAR) {
        // unescaping a single variable -- 
        // change escaping status of that variable
        context->program->back().value.integerValue = 0; //noescape
    } else {
        // other function call -- generate code for it
        ParserValue_t val;
        val.stringValue = name; //function name
        val.integerValue = nparams; //number of args
        tengCode_generate(context, Instruction_t::FUNC, val);
    }
}


/** Generate byte-code for printing a value.
  * @param context A parser context. */
void tengCode_generatePrint(ParserContext_t *context)
{
    // get actual program size
    unsigned int prgsize = context->program->size();
    // if optimalization does not step across opt limit address
    if (prgsize >= 3 //overflow protect
            && prgsize - 3 >= context->lowestValPrintAddress
            && (*context->program)[prgsize - 1].operation
            == Instruction_t::VAL //last op.
            && (*context->program)[prgsize - 2].operation
            == Instruction_t::PRINT
            && (*context->program)[prgsize - 3].operation
            == Instruction_t::VAL) {
        // optimalize sequence of VAL, PRINT, VAL, PRINT
        // to a single VAL, PRINT pair
        (*context->program)[prgsize - 3].value.setString(
                (*context->program)[prgsize - 3].value.stringValue
                + (*context->program)[prgsize - 1].value.stringValue);
        context->program->pop_back(); //delete last VAL instruction
    } else {
        // no way, simply add print instruction
        tengCode_generate(context, Instruction_t::PRINT);
    }
}


/** Optimize byte code for static expressions.
  * Expressions are examined for evaluation (from given start to end of prog)
  * and on success result is substituted instead of tested expression code.
  * @param context A parser context.
  * @param start Expressions starting address within program. */
void tengCode_optimizeExpression(ParserContext_t *context,
        unsigned int start)
{
    // try to evaluate given part of prog
    ParserValue_t val;
    int rc = context->evalProcessor->eval(
            val, start, context->program->size());
    // subst code on success
    if (rc == 0) {
        // remove expression's program
        context->program->erase(
                context->program->begin() + start,
                context->program->end());
        // code value
        tengCode_generate(context, Instruction_t::VAL, val);
    }
}

} // namespace Teng
