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

#ifndef TENGSEMANTICOTHER_H
#define TENGSEMANTICOTHER_H

#include <string>

#include "semantic.h"

namespace Teng {
namespace Parser {

/** Opens given file and replace include directive with content of the file.
 */
void include_file(Context_t *ctx, const Pos_t &pos, const Options_t &opts);

/** Writes warning into log error about ignored include.
 */
void ignore_include(Context_t *ctx, const Token_t &token, bool empty = false);

/** Generates warning with unknown Teng directive message.
 */
void ignore_unknown_directive(Context_t *ctx, const Token_t &token);

/** Generates warning about excessive options in directive that does not accept
 * any.
 */
void ignore_excessive_options(Context_t *ctx, const Pos_t &pos);

/** Inserts new option to options list. It expects that ctx->opts_sym is valid
 * symbol (not moved out).
 */
void new_option(Context_t *ctx, const Token_t &name, Literal_t &&literal);

/** Generates instructions implementing the function call.
 */
uint32_t generate_func(Context_t *ctx, const Token_t &name, uint32_t nargs);

} // namespace Parser
} // namespace Teng

#endif /* TENGSEMANTICOTHER_H */

