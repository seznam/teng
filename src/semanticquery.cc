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

#include "syntax.hh"
#include "logging.h"
#include "instruction.h"
#include "semanticexpr.h"
#include "semanticvar.h"
#include "semanticquery.h"

#ifdef DEBUG
#include <iostream>
#define DBG(...) __VA_ARGS__
#else /* DEBUG */
#define DBG(...)
#endif /* DEBUG */

namespace Teng {
namespace Parser {

void generate_query(Context_t *ctx, const Variable_t &var, bool warn) {
    generate<LogSuppress_t>(ctx, var.pos);
    note_optimization_point(ctx, true);
    generate_var(ctx, var, false);

    // warn if query is name($some.var)
    if (warn) {
        logWarning(
            ctx,
            var.pos,
            "In query expression the identifier shouldn't be denoted by $ sign"
        );
    }
}

/** Generates instructions implementing query expression.
 * If arity is not 1 then query is badly formated and instruction is not
 * generated.
 */
template <typename Instr_t>
NAryExpr_t query_expr(Context_t *ctx, const Token_t &token, uint32_t arity) {
    if (std::is_same<Instr_t, QueryDefined_t>::value) {
        logWarning(
            ctx,
            token.pos,
            "The defined() query is deprecated; "
            "use isempty() or exists() instead"
        );
    } else if (std::is_same<Instr_t, QueryCount_t>::value) {
        logWarning(
            ctx,
            token.pos,
            "The count() query is deprecated; "
            "use _count builtin variable instead"
        );
    }
    if (arity == 1) {
        generate<Instr_t>(ctx, token.pos);
    } else {
        logError(
            ctx,
            token.pos,
            "Invalid variable identifier in " + token.view() + "()"
        );
        generate_val(ctx, token.pos, Value_t());
    }
    return NAryExpr_t(token, arity);
}

// instatiations
template
NAryExpr_t query_expr<QueryRepr_t>(Context_t *, const Token_t &, uint32_t);
template
NAryExpr_t query_expr<QueryDefined_t>(Context_t *, const Token_t &, uint32_t);
template
NAryExpr_t query_expr<QueryExists_t>(Context_t *, const Token_t &, uint32_t);
template
NAryExpr_t query_expr<QueryIsEmpty_t>(Context_t *, const Token_t &, uint32_t);
template
NAryExpr_t query_expr<QueryType_t>(Context_t *, const Token_t &, uint32_t);
template
NAryExpr_t query_expr<QueryCount_t>(Context_t *, const Token_t &, uint32_t);

} // namespace Parser
} // namespace Teng

