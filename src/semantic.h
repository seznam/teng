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

#ifndef TENGSEMANTIC_H
#define TENGSEMANTIC_H

#include <utility>

namespace Teng {

// forwards
struct Pos_t;
class Regex_t;
class Value_t;
class Identifier_t;

namespace Parser {

// forwards
class Token_t;
struct Context_t;
class Options_t;
class Literal_t;
class Variable_t;
class NAryExpr_t;
struct VarOffset_t;
enum class diag_code;

/** Saves error position for further processing.
 */
Token_t note_error(Context_t *ctx, const Token_t &token);

/** Clears last stored error.
 */
void reset_error(Context_t *ctx);

/** Generates instruction for given value.
 */
void generate_val(Context_t *ctx, const Pos_t &pos, Value_t value);

/** Inserts new diagnostic code into diag-codes storage. If pop is set to true
 * then the previous diagnostic code is poped out.
 *
 * The diagnostic codes are used to help the template writer where the syntax
 * error probably is.
 */
void expr_diag(Context_t *ctx, diag_code new_diag_code, bool pop = true);

/** Inserts new diagnostic code into diag-codes storage including the diag code
 * sentinel.
 */
void expr_diag_sentinel(Context_t *ctx, diag_code new_diag_code);

/** Generates given instruction pass given args to instruction c'tor.
 */
template <typename Instr_t, typename Ctx_t, typename... Args_t>
void generate(Ctx_t *ctx, Args_t &&...args) {
    ctx->program->template emplace_back<Instr_t>(std::forward<Args_t>(args)...);
}

} // namespace Parser
} // namespace Teng

#endif /* TENGSEMANTIC_H */

