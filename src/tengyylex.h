/*
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
 * $Id$
 *
 * DESCRIPTION
 * Teng syntax analyzer.
 *
 * AUTHORS
 * Michal Bukovsky
 *
 * HISTORY
 * 2018-05-07  (burlog)
 *             Adapted from tengsyntax.yy
 */

#ifndef TENGYYLEX_H
#define TENGYYLEX_H

namespace Teng {

struct LeftValue_t;
struct ParserContext_t;

/** Lexical analyzer.
 *
  * Calls preprocessor (lex level #1) and then lexical analyzer (lex level #2)
  * for some level #1 tokens.
  *
  * @param leftValue Pointer to the token's left-value.
  * @param context Pointer to the parser's control structure.
  *
  * @return 0=EOF, >0=element identificator.
  */
int yylex(LeftValue_t *leftValue, ParserContext_t *context);

} // namespace Teng

#endif // TENGYYLEX_H

