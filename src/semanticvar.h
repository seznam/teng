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

#ifndef TENGSEMANTICVAR_H
#define TENGSEMANTICVAR_H

#include <string>

#include "semantic.h"

namespace Teng {
namespace Parser {

/** Generates code for variable.
 */
void generate_var(Context_t *ctx, Variable_t var, bool gen_repr = true);

/** Prepare Variable_t symbol in context var_sym temporary for absolute
 * variable that starts with '.';
 */
void prepare_root_variable(Context_t *ctx, const Token_t &token);

/** Prepare Variable_t symbol in context var_sym temporary for absolute
 * variable that starts with '_this';
 */
void prepare_this_variable(Context_t *ctx, const Token_t &token);

/** Prepare Variable_t symbol in context var_sym temporary for absolute
 * variable that starts with '_parent';
 */
void prepare_parent_variable(Context_t *ctx, const Token_t &token);

/** Generates code implementing the setting variable.
 */
void set_var(Context_t *ctx, Variable_t var_name);

/** Generates warning about invalid variable name.
 */
void ignore_inv_set(Context_t *ctx, const Pos_t &pos);

/** Generates code implementing runtime variable indexing.
 */
void generate_rtvar_index(Context_t *ctx, const Token_t &lp, const Token_t &rp);

/** Generates instructions implementing getting value of runtime variable for
 * desired key.
 */
void generate_rtvar_segment(Context_t *ctx, const Token_t &token, bool is_first);

/** Generates instructions implementing getting this fragment.
 */
void generate_rtvar_this(Context_t *ctx, const Token_t &token, bool is_first);

/** Generates instructions implementing getting parent fragment.
 */
void generate_rtvar_parent(Context_t *ctx, const Token_t &token, bool is_first);

/** Generates instructions implementing local runtime variable.
 */
void generate_local_rtvar(Context_t *ctx, const Token_t &token);

/** Generates instructions implementing runtime variable root.
 */
void generate_rtvar_root(Context_t *ctx, const Token_t &token);

/** Generates nice warning abount ignored _this.
 */
void ignoring_this(Context_t *ctx, const Pos_t &pos);

/** Generates nice warning abount ignored dollar.
 */
void obsolete_dollar(Context_t *ctx, const Pos_t &pos);

/** Attemps resolve relative variable in any open frames in reverse order. It
 * does not do _ANY_ check that variable is relative. The examples explains
 * resolution better than long explanations.
 *
 * frames: .a.b.c, .a.d, .a.b
 *
 * then ident b.x refers to .a.b.x in current frame
 * then ident a.b.x refers to .a.b.x in current frame
 * then ident a.x refers to .a.x in current frame
 *
 * then ident d.x refers to .a.d.x in -1 frame
 * then ident a.d.x refers to .a.d.x in -1 frame
 *
 * then ident c.x refers to .a.b.c.x in -2 frame
 * then ident a.b.c.x refers to .a.b.c.x in -2 frame
 */
VarOffset_t resolve_relative_var(Context_t *ctx, const Variable_t &var_sym);

/** Absolute vars can refer to variables in any open frame and open fragment
 * but not to closed one.
 */
VarOffset_t resolve_abs_var(Context_t *ctx, const Variable_t &var_sym);

/** Converts relative variable to absolute.
 *
 * Assumption: var_sym offsets are valid
 */
Identifier_t make_absolute_ident(Context_t *ctx, const Variable_t &var_sym);

/** Inserts address of expression in brackets to stack of index start points.
 */
void note_rtvar_index_start_point(Context_t *ctx);

/** Cleans after generating runtime variable.
 */
void rtvar_clean(Context_t *ctx);

} // namespace Parser
} // namespace Teng

#endif /* TENGSEMANTICVAR_H */

