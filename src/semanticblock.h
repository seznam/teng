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

#ifndef TENGSEMANTICBLOCK_H
#define TENGSEMANTICBLOCK_H

#include <string>

#include "semantic.h"

namespace Teng {
namespace Parser {

/** Generates instructions implementing format switching.
 */
void open_format(Context_t *ctx, const Pos_t &pos, const Options_t &opts);

/** Generates instructions implementing format switching.
 */
void open_inv_format(Context_t *ctx, const Pos_t &pos);

/** Generates instructions that restore previous (or default) format.
 */
void close_format(Context_t *ctx, const Pos_t &pos);

/** Generates instructions that restore previous (or default) format and
 * reports invalid token.
 */
void close_inv_format(Context_t *ctx, const Pos_t &pos);

/** Generates instructions that restore previous (or default) format and
 * reports missing close format directive.
 */
void
close_unclosed_format(Context_t *ctx, const Pos_t &pos, const Token_t &token);

/** Generates instructions implementing content type switching.
 */
void open_ctype(Context_t *ctx, const Pos_t &pos, const Literal_t &type);

/** Generates instructions implementing content type switching.
 */
void open_inv_ctype(Context_t *ctx, const Pos_t &pos);

/** Generates instructions that restore previous (or default) content type.
 */
void close_ctype(Context_t *ctx, const Pos_t &pos);

/** Generates instructions that restore previous (or default) content type and
 * reports invalid token.
 */
void close_inv_ctype(Context_t *ctx, const Pos_t &pos);

/** Generates instructions that restore previous (or default) content type and
 * reports missing close content type directive.
 */
void
close_unclosed_ctype(Context_t *ctx, const Pos_t &pos, const Token_t &token);

} // namespace Parser
} // namespace Teng

#endif /* TENGSEMANTICBLOCK_H */

