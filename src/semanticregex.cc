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

#include "regex.h"
#include "syntax.hh"
#include "logging.h"
#include "instruction.h"
#include "parsercontext.h"
#include "semanticregex.h"

#ifdef DEBUG
#include <iostream>
#define DBG(...) __VA_ARGS__
#else /* DEBUG */
#define DBG(...)
#endif /* DEBUG */

namespace Teng {
namespace Parser {

counted_ptr<Regex_t> generate_regex(Context_t *ctx, const Token_t &regex) {
    regex_flags_t flags;
    auto i = regex.view().size() - 1;
    for (; regex.view()[i] != '/'; --i) {
        switch (regex.view()[i]) {
        case 'i': flags->ignore_case = true; break;
        case 'g': flags->global = true; break;
        case 'm': flags->multiline = true; break;
        case 'e': flags->extended = true; break;
        case 'X': flags->extra = true; break;
        case 'U': flags->ungreedy = true; break;
        case 'A': flags->anchored = true; break;
        case 'D': flags->dollar_endonly = true; break;
        default:
            logWarning(
                ctx,
                regex.pos,
                "Ignoring unknown regex flag '"
                + std::string(1, regex.view()[i])
                + "'"
            );
        }
    }

    try {
        auto *data = regex.view().data();
        string_view_t pattern = {data + 1, data + i};
        return make_counted<Regex_t>(pattern, flags);

    } catch (const std::exception &e) {
        logWarning(
            ctx,
            regex.pos,
            std::string("Invalid regular expression: ") + e.what()
        );
    }
    return make_counted<Regex_t>(".^", flags);
}

void
generate_match(Context_t *ctx, const Token_t &token, const Token_t &regex) {
    // parse regex (split pattern and flags)
    auto regex_value = generate_regex(ctx, regex);

    // generate instruction for parsed regex
    generate<MatchRegex_t>(ctx, std::move(regex_value), regex.pos);
    if (token == LEX2::STR_NE)
        generate<Not_t>(ctx, token.pos);
}

} // namespace Parser
} // namespace Teng

