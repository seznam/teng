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

#ifndef TENGSEMANTICIF_H
#define TENGSEMANTICIF_H

#include <string>

#include "semantic.h"

namespace Teng {
namespace Parser {

/** Prepares new if statement.
 */
void prepare_if_stmnt(Context_t *ctx, const Pos_t &pos);

/** Cleans after if statement building.
 */
void finalize_if_stmnt(Context_t *ctx);

/** Generates instructions implementing the if expression.
 */
void generate_if(Context_t *ctx, const Token_t &token, bool valid_expr);

/** Generates instructions implementing the if expression.
 */
void generate_if(Context_t *ctx, const Token_t &token, const Token_t &inv);

/** Updates branch terminating jumps.
 */
void generate_endif(Context_t *ctx, const Pos_t *inv_pos = nullptr);

/** Cleans after invalid if statement building.
 */
void finalize_inv_if_stmnt(Context_t *ctx, const Token_t &token);

/** Generates instructions implementing the else expression.
 */
void generate_else(Context_t *ctx, const Token_t &token, bool invalid = false);

/** Calculates if expression's jump and updates appropriate instructions in
 * program.
 */
void prepare_elif(Context_t *ctx, const Token_t &token);

/** Discards while if statement because of invalid order of else/elif branches.
 */
void discard_if_stmnt(Context_t *ctx);

/** Generates instructions implementing the if expression.
 */
inline void
generate_elif(Context_t *ctx, const Token_t &token, bool valid_expr) {
    generate_if(ctx, token, valid_expr);
}

/** Generates instructions implementing the if expression.
 */
inline void
generate_elif(Context_t *ctx, const Token_t &token, const Token_t &inv) {
    generate_if(ctx, token, inv);
}

} // namespace Parser
} // namespace Teng

#endif /* TENGSEMANTICIF_H */


