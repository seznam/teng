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

#ifndef TENGSEMANTICINHERITANCE_H
#define TENGSEMANTICINHERITANCE_H

#include <string>

#include "semantic.h"

namespace Teng {
namespace Parser {

/** Compiles the base template which this one extends.
 */
void extends_file(Context_t *ctx, const Pos_t &pos, const Options_t &opts);

/** Writes warning into log error about ignored extends.
 */
void ignore_extends(Context_t *ctx, const Token_t &token, bool empty = false);

/** Close list of overriding blocks.
 */
void close_extends(Context_t *ctx, const Pos_t *inv_pos = nullptr);

/** Starts overridden block.
 */
void note_override_block(Context_t *ctx, const Token_t &ident);

/** Registers overridden block into the overridden block registry of this
 * extends block.
 */
void reg_override_block(Context_t *ctx, const Token_t &end_block);

/** Starts generating the base implementation of this overridden block.
 */
void note_define_block(Context_t *ctx, const Token_t &ident);

/** Warns that this define block token is invalid.
 */
void note_inv_define_block(Context_t *ctx, const Token_t &define_block);

/** Registers overridden block into the overridden block registry of this
 * extends block and starts generating the overrides.
 */
void reg_define_block(Context_t *ctx, const Token_t &end_block);

/** Generates CALL instruction that calls the parent block implementation.
 */
void call_super_block(Context_t *ctx, const Token_t &super_block);

/** The override directive out of extends block.
 */
void ignore_free_override(Context_t *ctx, const Token_t &token);

/** Tells the user that extends block is unclosed.
 */
void ignore_unclosed_extends(Context_t *ctx, const Token_t &token);

/** Warns that this override block token is invalid.
 */
void note_inv_override_block(Context_t *ctx, const Token_t &override_block);

/** Closes unclosed define block and generate warning.
 */
void unclosed_define_block(Context_t *ctx, const Token_t &token);

} // namespace Parser
} // namespace Teng

#endif /* TENGSEMANTICINHERITANCE_H */

