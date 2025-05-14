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
 * Teng syntax symbols.
 *
 * AUTHORS
 * Filip Volejnik <filip.volejnik@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz
 *
 * HISTORY
 * 2012-04-16  (volejnik)
 *             First version.
 * 2018-07-07  (burlog)
 *             Cleaned.
 */

#include "parsercontext.h"
#include "logging.h"
#include "yystype.h"

namespace Teng {
namespace Parser {

Variable_t &Variable_t::pop_back(Context_t *ctx, const Pos_t &pos) {
    if (!ident.empty()) ident.pop_back();
    else logWarning(ctx, pos, "The _parent violates the root boundary");
    return *this;
}

/** Converts the view to the string value.
 */
std::string Literal_t::extract_str(Context_t *ctx, const Token_t &token) {
    // remove quotes
    auto raw = string_view_t(token.view().begin() + 1, token.view().end() - 1);

    // replace escape sequences
    std::string result;
    result.reserve(raw.size());
    for (auto iraw = raw.begin(), eraw = raw.end(); iraw != eraw;) {
        switch (*iraw) {
        case '\\':
            if (++iraw == eraw) {
                // It is impossible to get here with ++iraw
                // equaling to eraw because '...\' backslach preceding closing
                // quote is taken as escape sequence and so the closing quote
                // is not closing quote but escaped quote and string
                // continues...
                logWarning(ctx, token.pos, "Trailing backslash in string");
                return result;
            }
            switch (*iraw) {
            case '\n':
                logWarning(ctx, token.pos, "Newline can't be escaped");
                return result;
            case 'r':
                result.push_back('\r');
                ++iraw;
                break;
            case 'n':
                result.push_back('\n');
                ++iraw;
                break;
            case 't':
                result.push_back('\t');
                ++iraw;
                break;
            case 'f':
                result.push_back('\f');
                ++iraw;
                break;
            case 'b':
                result.push_back('\b');
                ++iraw;
                break;
            default:
                result.push_back(*iraw);
                ++iraw;
                break;
            }
            break;
        default:
            result.push_back(*iraw);
            ++iraw;
            break;
        }
    }
    return result;
}

} // namespace Parser
} // namespace Teng

