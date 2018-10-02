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
 * $Id: tengutf8.cc,v 1.18 2008-11-20 23:32:29 burlog Exp $
 *
 * DESCRIPTION
 * Utf8 support in teng.
 *
 * AUTHORS
 * Jan Nemec <jan.nemec@firma.seznam.cz>
 * Stepan Skrob <stepan@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-06-14  (burlog)
 *             Moved from tengfunction.cc.
 */

#ifndef TENGUTF8_H
#define TENGUTF8_H

#include <string>

#include "tengstringview.h"

namespace Teng {

// forwards
struct Regex_t;

namespace utf8 {

/** Returns bytes length of the UTF-8 character.
 */
std::size_t charlen(char ch);

/** Strlen for UTF-8 string.
 * @param str string
 * @return length of str
 */
std::size_t strlen(const string_view_t &str);

/** Python-like substr for UTF-8 string.
 * @param str source string
 * @param s start index
 * @param e end index
 */
std::string
substr(const string_view_t &str, int s, int e, std::string p1, std::string p2);

/** Find real indexes into string for UTF-8 substr.
 * @param str source string
 * @param s start index
 * @param e end index
 */
void substr(const string_view_t &str, int &s, int &e);

/** Converts utf-8 string to lowercase.S
 */
std::string tolower(const string_view_t &str);

/** Converts utf-8 string to uppercase.
 */
std::string toupper(const string_view_t &str);

} // namespace utf8
} // namespace Teng

#endif /* TENGUTF8_H */

