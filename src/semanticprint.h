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

#ifndef TENGSEMANTICPRINT_H
#define TENGSEMANTICPRINT_H

#include <string>

#include "semantic.h"

namespace Teng {
namespace Parser {

/** Generates print instruction.
 */
void generate_print(Context_t *ctx, bool print_escape = true);

/** Generates lookup to dictionary instruction.
 */
void generate_dict_lookup(Context_t *ctx, const Token_t &token);

/** Generates raw print instruction.
 */
void generate_raw_print(Context_t *ctx);

/** Generates print instruction of given token value.
 */
void generate_raw_print(Context_t *ctx, const Token_t &token);

/** Generates print instruction of given token value. It expects that value is
 * "undefined" which does not need escaping.
 */
void generate_inv_print(Context_t *ctx, const Token_t &inv);

/** Generates lookup to dictionary instruction.
 */
void print_dict_lookup(Context_t *ctx, const Token_t &token);

/** Generates undefined value due to invalid dict identifier.
 */
void print_dict_undef(Context_t *ctx, const Token_t &token);

} // namespace Parser
} // namespace Teng

#endif /* TENGSEMANTICPRINT_H */

