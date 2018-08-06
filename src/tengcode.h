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
 * $Id: tengcode.h,v 1.2 2004-12-30 12:42:01 vasek Exp $
 *
 * DESCRIPTION
 * Teng byte-code generation.
 *
 * AUTHORS
 * Stepan Skrob <stepan@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-30  (stepan)
 *             Created.
 */

#ifndef TENGCODE_H
#define TENGCODE_H

#include "tengparsercontext.h"
#include "tenginstruction.h"
#include "tengparservalue.h"


namespace Teng {

/** Generate byte-code for general instruction.
  * @param context A parser context.
  * @param code Instruction code.
  * @param value Optional instruction parameter value(s). */
void generateCode(ParserContext_t *context,
                  Instruction_t::OpCode_t code,
                  const ParserValue_t &value = ParserValue_t());

/** Generate byte-code for a function call.
  * Also optimize 'unescape($variable)' call.
  * @param context A parser context.
  * @param name The function name.
  * @param narams Number of params in the call. */
void generateFunctionCall(ParserContext_t *context,
                          const std::string &name, int nparams);

/** Generate byte-code for printing a value.
  * @param context A parser context. */
void generatePrint(ParserContext_t *context);

/** Optimize byte code for static expressions.
  * Expressions are examined for evaluation (from given start to end of prog)
  * and on success result is substituted instead of tested expression code.
  * @param context A parser context.
  * @param start Expressions starting address within program. */
void optimizeExpression(ParserContext_t *context,
                        unsigned int start);

} // namespace Teng

#endif // TENGCODE_H
