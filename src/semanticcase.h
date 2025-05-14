/* !don't remove! -*- C++ -*-
 *
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
 * $Id: tengsyntax.yy,v 1.14 2010-06-11 08:25:35 burlog Exp $
 *
 * DESCRIPTION
 * Teng grammar semantic actions.
 *
 * AUTHORS
 * Stepan Skrob <stepan@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-06-07  (burlog)
 *             Moved from syntax.yy.
 */

#ifndef TENGSEMANTICCASE_H
#define TENGSEMANTICCASE_H

#include <string>

#include "semantic.h"

namespace Teng {
namespace Parser {

/** Prepares case expression.
 */
void prepare_case(Context_t *ctx);

/** Prepares condition of the case expression.
 */
void prepare_case_cond(Context_t *ctx, const Token_t &token);

/** Generates comparison for case branch.
 */
uint32_t generate_case_cmp(Context_t *ctx, Literal_t &literal);

/** Updates jmp address offset in Jmp_t instruction generated for case
 * alternative value.
 */
void update_case_jmp(Context_t *ctx, const Token_t &token, uint32_t alts);

/** Generates alternative matching instructions for case option alternative.
 */
uint32_t generate_case_next(Context_t *ctx, Literal_t &literal, uint32_t alts);

/** Updates jmp address offset in JmpIfNot_t instruction generated for result
 * of matching case option.
 */
void finalize_case_branch(Context_t *ctx, const Token_t &token);

/** The case expression is a bit complicated so it takes a lot instruction to
 * build it. Consider such expression and, for the sake of simplicity, split it
 * to several parts.
 *
 * case(1, 1: 'first', 2, 3: 'second', *: 'default')
 *
 * 1. The case expression condition
 *
 * The condition is an expression, here numeric literal 1, that has to be pushed
 * onto value stack:
 *
 * 000 VAL                 <value=1>
 * 001 PUSH
 *
 * 2. The label of first case branch
 *
 * The case label has to be literal and should be compared with case
 * 'condition' value. These instructions implement such algorithm.
 *
 * 002 STACK               <index=0>
 * 003 VAL                 <value=1>
 * 004 EQ
 * 005 JMPIFNOT            <jump=-1>
 *
 * The STACK instruction push the top of value stack onto compute stack. The EQ
 * takes two values from compute stack and pushes back the result of comparison.
 * If comparison returns true then JMPIFNOT instruction does not perform jump
 * and case branch for current label is executed.
 *
 * 006 VAL                 <value='first'>
 * 007 JMP                 <jump=-1>
 *
 * The last instruction is jump at the end of case expr.
 *
 * 3. The label of second case branch
 *
 * The second label has two alternatives. They are implementated as ORing of
 * two comparison as in previous label.
 *
 * 008 STACK               <index=0>
 * 009 VAL                 <value=2>
 * 010 EQ
 * 011 OR                  <jump=+4>
 * 012 STACK               <index=0>
 * 013 VAL                 <value=3>
 * 014 EQ
 * 015 JMPIFNOT            <jump=-1>
 * 016 VAL                 <value='second'>
 * 017 JMP                 <jump=-1>
 *
 * 4. The default label of case
 *
 * If one label match the case condition value then the instruction in default
 * branch are executed.
 *
 * 018 VAL                 <value='ostatni'>
 *
 * 5. Finalizing
 *
 * You could noticed that jump instruction has set invalid offsets. They can't
 * be calculated during generating instructions because the offset aren't
 * known at the time. Therefore, the ctx->branch_addrs stack of jpm
 * instructions is used to store the addresses of all instructions, and the
 * offsets are immediately updated as they are known.
 *
 * Finally, the case condition value has to be pop out from value stack.
 *
 * 019 POP
 */
NAryExpr_t
finalize_case(Context_t *ctx, const Token_t &token, uint32_t arity);

} // namespace Parser
} // namespace Teng

#endif /* TENGSEMANTICCASE_H */

