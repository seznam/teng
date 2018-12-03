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
 * $Id: tengfunction.cc,v 1.18 2008-11-20 23:32:29 burlog Exp $
 *
 * DESCRIPTION
 * Teng processor function (like len, substr, round or date)
 *
 * AUTHORS
 * Jan Nemec <jan.nemec@firma.seznam.cz>
 * Stepan Skrob <stepan@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-26  (honza)
 *             Created.
 * 2004-05-08  (vasek)
 *             Polished code.
 * 2005-06-21  (roman)
 *             Win32 support.
 * 2018-06-14  (burlog)
 *             Polished.
 */

#ifndef TENGFUNCTIONESCAPING_H
#define TENGFUNCTIONESCAPING_H

#include <string>
#include <sstream>

#include "hex.h"
#include "platform.h"
#include "functionutil.h"
#include "contenttype.h"
#include "function.h"

namespace Teng {
namespace builtin {

/** Escapes string. Format depends on escaper.
 *
 * @param args Teng function arguments
 * @param ctx Teng function ctx
 * @param result Teng function result
 */
Result_t escape(Ctx_t &ctx, const Args_t &args) {
    return args.size() != 1
        ? wrongNumberOfArgs(ctx, "escape", 1)
        : Result_t(args[0].print([&] (const string_view_t &arg) {
            return ctx.escaper().escape(arg);
        }));
}

/** Unescapes string. Format depends on escaper.
 *
 * @param args Teng function arguments
 * @param ctx Teng function ctx
 * @param result Teng function result
 */
Result_t unescape(Ctx_t &ctx, const Args_t &args) {
    return args.size() != 1
        ? wrongNumberOfArgs(ctx, "unescape", 1)
        : Result_t(args[0].print([&] (const string_view_t &arg) {
            return ctx.escaper().unescape(arg);
        }));
}

/** Escape disallowed chars in URL arguments.
 *
 * @param args Function arguments (list of values).
 * @param ctx Teng function ctx.
 * @param result Function's result value.
 */
Result_t urlescape(Ctx_t &ctx, const Args_t &args) {
    if (args.size() != 1)
        return wrongNumberOfArgs(ctx, "urlescape", 1);
    return Result_t(args[0].print([] (const string_view_t &arg) {
        static const auto hex = "0123456789ABCDEF";
        std::string escaped;
        for (auto ch: arg) {
            switch (ch) {
            case 0x21:
            case 0x3d:
            case 0x24 ... 0x3b:
            case 0x3e ... 0x7e:
                escaped.push_back(ch);
                break;
            default:
                escaped.push_back('%');
                escaped.push_back(hex[uint8_t(ch) >> 4]);
                escaped.push_back(hex[uint8_t(ch) & 0x0f]);
                break;
            }
        }
        return escaped;
    }));
}

/** Unescape url - reverse urlescape function
 *
 * @param args Function arguments (list of values).
 * @param ctx Teng function ctx.
 * @param result Function's result value.
 */
Result_t urlunescape(Ctx_t &ctx, const Args_t &args) {
    if (args.size() != 1)
        return wrongNumberOfArgs(ctx, "urlunescape", 1);
    return Result_t(args[0].print([] (const string_view_t &arg) {
        std::string unescaped;
        for (auto ipos = arg.begin(), epos = arg.end(); ipos != epos;) {
            if (*ipos == '%') {
                auto itmp = ipos;
                if (++itmp != epos) {
                    auto first = *itmp;
                    if (++itmp != epos) {
                        auto second = *itmp;
                        if (isxdigit(first) && isxdigit(second)) {
                            auto byte = unhex(uint8_t(first), uint8_t(second));
                            unescaped.push_back(byte);
                            ipos = ++itmp;
                            continue;
                        }
                    }
                }
            }
            unescaped.push_back(*ipos++);
        }
        return unescaped;
    }));
}

/** Create quotable string.
 *
 * @param args Function arguments (list of values).
 * @param ctx Teng function ctx.
 * @param result Function's result value.
 */
Result_t quoteescape(Ctx_t &ctx, const Args_t &args) {
    if (args.size() != 1)
        return wrongNumberOfArgs(ctx, "quoteescape", 1);
    return Result_t(args[0].print([] (const string_view_t &arg) {
        std::string escaped;
        for (auto ch: arg) {
            switch (ch) {
            case '\\': escaped.append("\\\\"); break;
            case '\n': escaped.append("\\n"); break;
            case '\r': escaped.append("\\r"); break;
            case '\a': escaped.append("\\a"); break;
            case '\0': escaped.append("\\0"); break;
            case '\v': escaped.append("\\v"); break;
            case '\'': escaped.append("\\'"); break;
            case '"': escaped.append("\\\""); break;
            default: escaped.push_back(ch); break;
            }
        }
        return escaped;
    }));
}

/** Converts value to json.
 *
 * @param args Function arguments (list of values).
 * @param ctx Teng function ctx.
 * @param result Function's result value.
 */
Result_t jsonify(Ctx_t &ctx, const Args_t &args) {
    if (args.size() != 1)
        return wrongNumberOfArgs(ctx, "jsonify", 1);
    std::stringstream out;
    args[0].json(out);
    return Result_t(out.str());
}

} // namespace builtin
} // namespace Teng

#endif /* TENGFUNCTIONESCAPING_H */

