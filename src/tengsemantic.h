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
 qstart* Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-06-07  (burlog)
 *             Moved from syntax.yy.
 */

#ifndef TENGSEMANTIC_H
#define TENGSEMANTIC_H

// includes
#include <string>

#include "tengcode.h"
#include "tengyylex.h"
#include "tengerror.h"
#include "tenglogging.h"
#include "tengyystype.h"
#include "tenginstruction.h"

namespace Teng {
namespace Parser {

/** @short Generate code for variable.
 * @param result use $$ as this parameter
 * @param val value with variable name ($x).
 */
void
codeForVariable(Context_t *ctx, Symbol_t &result, const Symbol_t &name);

/** 
 */
void
buildLocVar(Context_t *ctx, Symbol_t &result, Symbol_t &name);

/** 
 */
void
pushVarName(
    Context_t *ctx,
    Symbol_t &result,
    const Symbol_t &statement,
    const Symbol_t &name
);

/** 
 */
void
buildAbsVar(Context_t *ctx, Symbol_t &result, const Symbol_t &statement);

/** 
 */
void popAbsVarSegment(Context_t *ctx, const Symbol_t &segment);

/** 
 */
void pushAbsVarSegment(Context_t *ctx, const Symbol_t &segment);

/** 
 */
void
buildRelVar(Context_t *ctx, Symbol_t &result, const Symbol_t &statement);

/** 
 */
void pushRelVarSegment(Context_t *ctx, const Symbol_t &segment);

/** 
 */
void
includeFile(Context_t *ctx, const Symbol_t &incl, const Symbol_t &opts);

/** 
 */
void
openFormat(
    Context_t *ctx,
    Symbol_t &result,
    const Symbol_t &format,
    const Symbol_t &opts
);

/** 
 */
void closeFormat(Context_t *ctx, const Symbol_t &format);

/** 
 */
void openFrag(Context_t *ctx, Symbol_t &result, const Symbol_t &name);

/** 
 */
void closeFrag(Context_t *ctx, const Symbol_t &name);

/** 
 */
void
setVariable(
    Context_t *ctx,
    Symbol_t &result,
    const Symbol_t &statement,
    const Symbol_t &name
);

/** 
 */
void buildIfEndJump(Context_t *ctx, const Symbol_t &, const Symbol_t &, const Symbol_t &);

/** 
 */
void buildElseEndJump(Context_t *ctx, Symbol_t &, const Symbol_t &, const Symbol_t &, const Symbol_t &, const Symbol_t &);

/** 
 */
std::size_t
finalizeBinOp(Context_t *ctx, const Symbol_t &, const Symbol_t &);

/** 
 */
std::size_t buildTernOp(Context_t *ctx, const Symbol_t &true_expr);

/** 
 */
std::size_t
finalizeTernOp(Context_t *ctx, const Symbol_t &lhs, const Symbol_t &rhs);

/** 
 */
std::size_t
generateUndefined(
    Context_t *ctx,
    Symbol_t &result,
    const Symbol_t &statement
);

/** 
 */
Value_t openCType(Context_t *ctx, const Symbol_t &);

/** 
 */
void closeCType(Context_t *ctx, const Symbol_t &);

/** 
 */
void openRepeat(Context_t *ctx, const Symbol_t &);

/** 
 */
void generateRepr(Context_t *ctx);

/** 
 */
void generateRepr(Context_t *ctx, const Symbol_t &op);

/** 
 */
void
generateDictLookup(
    Context_t *ctx,
    Symbol_t &result,
    const Symbol_t &ident
);

/** 
 */
void
finalizeCaseOp(
    Context_t *ctx,
    Symbol_t &result,
    const Symbol_t &case_start,
    const Symbol_t &case_options
);

/** 
 */
void
finalizeCaseOptionsOp(
    Context_t *ctx,
    Symbol_t &result,
    const Symbol_t &matching_branch_start,
    const Symbol_t &matching_branch_end,
    const Symbol_t &next_option
);

/** 
 */
void
generateCaseLiteral(
    Context_t *ctx,
    Symbol_t &result,
    const Symbol_t &case_value
);

/** 
 */
void generateCaseValues(
    Context_t *ctx,
    Symbol_t &result,
    const Symbol_t &case_value
);

/** 
 */
void
generateQueryInstr(
    Context_t *ctx,
    Symbol_t &result,
    const Symbol_t &op,
    const Symbol_t &name
);

/** 
 */
void
prepareQueryInstr(
    Context_t *ctx,
    Symbol_t &result,
    const Symbol_t &name,
    const char *value
);

/** 
 */
std::size_t
generateFunction(
    Context_t *ctx,
    const Symbol_t &name,
    const Symbol_t &rparen,
    const Symbol_t &args
);

} // namespace Parser
} // namespace Teng

#endif /* TENGSEMANTIC_H */

