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

#ifndef TENGFUNCTIONOTHER_H
#define TENGFUNCTIONOTHER_H

#include <string>

#include <tengplatform.h>
#include <tengconfiguration.h>
#include <tengfunctionutil.h>
#include <tengfragmentstack.h>
#include <tengfunction.h>

namespace Teng {
namespace builtin {

/** Check whether given feature is enabled.
 *
 * @param args Function arguments (list of values).
 * @param ctx Teng function ctx.
 * @param result Function's result value.
 */
Result_t isenabled(Ctx_t &ctx, const Args_t &args) {
    if (args.size() != 1)
        return wrongNumberOfArgs(ctx, "isenabled", 1);

    auto &arg = args.back();
    if (!arg.is_string())
        return failed(ctx, "isenabled", "Arg must be a string");
    auto &feature = arg.as_str();

    bool enabled = false;
    if (ctx.configuration.isEnabled(feature, enabled)) {
        auto msg = "Unknown feature '" + feature + "'";
        return failed(ctx, "isenabled", msg);
    }

    return Result_t(enabled);
}

/** Check whether given key is present in dictionaries.
 *
 * @param args Function arguments (list of values).
 * @param ctx Teng function ctx.
 * @param result Function's result value.
 */
Result_t dictexist(Ctx_t &ctx, const Args_t &args) {
    if (args.size() != 1)
        return wrongNumberOfArgs(ctx, "dictexist", 1);

    auto &arg = args.back();
    if (!arg.is_string())
        return failed(ctx, "dictexist", "Arg must be a string");
    auto &key = arg.as_str();

    return Result_t(
        ctx.langDictionary.lookup(key) || ctx.configuration.lookup(key)
    );
}

/** Check whether given key is present in dictionaries.
 *
 * @param args Function arguments (list of values).
 * @param ctx Teng function ctx.
 * @param result Function's result value.
 */
Result_t getdict(Ctx_t &ctx, const Args_t &args) {
    if (args.size() != 2)
        return wrongNumberOfArgs(ctx, "getdict", 2);

    auto &arg0 = args[1];
    if (!arg0.is_string())
        return failed(ctx, "getdict", "First must be a string");
    auto &key = arg0.as_str();

    auto &arg1 = args[0];
    if (!arg1.is_string())
        return failed(ctx, "getdict", "Second must be a string");
    auto &def = arg1.as_str();

    // set result value
    if (auto *val = ctx.langDictionary.lookup(key))
        return Result_t(*val);
    if (auto *val = ctx.configuration.lookup(key))
        return Result_t(*val);
    return Result_t(def);
}

Result_t exists(Ctx_t &ctx, const Args_t &args) {
    for (auto &a: args) {
        std::cerr << a << std::endl;
    }

    std::cerr << "FRAGSTACK " << ctx.fragStack->currentPath() << std::endl;
    std::cerr << ctx.fragStack->chainSize() << std::endl;

    Identifier_t id;
//     // handle root fragment
//     if (name.empty()) {
//         // name is empty
//         id.name = std::string();
//         // current context (root is in all contexts)
//         id.context = fragContext.size() - 1;
//         // root is first
//         id.depth = 0;
//
//         // OK, found
//         return FR::FOUND;
//     }
//
//     // process all contexts and try to find varible's prefix (fragment name)
//     for (auto ifragctx = fragContext.rbegin();
//          ifragctx != fragContext.rend(); ++ifragctx)
//     {
//         if ((name.size() <= ifragctx->size())
//             && std::equal(name.begin(), name.end(),
//                           ifragctx->name.begin()))
//         {
//             // set object name
//             id.name = name.back();
//             // set context (how much to go from the root context)
//             id.context = (fragContext.rend() - ifragctx - 1);
//             // set fragment depth
//             id.depth = name.size();
//             return FR::FOUND;
//
//         } else if (parentIsOK // fragment name cannot be empty!
//                    && ((name.size() - 1) <= ifragctx->size())
//                    && std::equal(name.begin(), name.end() - 1,
//                                  ifragctx->name.begin()))
//         {
//             // we have found parent of requested fragment
//
//             // set object name
//             id.name = name.back();
//             // set context (how much to go from the root context)
//             id.context = (fragContext.rend() - ifragctx - 1);
//             // set fragment depth
//             id.depth = name.size() - 1;
//             return FR::PARENT_FOUND;
//         }
//     }
//
//     // log error (only when we are allowed to do so)
//     if (pos) {
//         logError(program->getErrors(), *pos,
//                  "Fragment '" + fullname(name) + "' not found in any context.");
//     }
//
//     // not found
//     return FR::NOT_FOUND;
// }

    // auto fr = findFragment(0, name, id, !mustBeOpen);
    // switch (ctx->exists(name.pos, result.id, fixed_name, id, mustBeOpen)) {
    return Result_t();
}

} // namespace builtin
} // namespace Teng

#endif /* TENGFUNCTIONOTHER_H */

