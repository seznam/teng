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

#include "platform.h"
#include "configuration.h"
#include "functionutil.h"
#include "function.h"

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
    if (!arg.is_string_like())
        return failed(ctx, "isenabled", "Arg must be a string");
    auto feature = arg.string();

    // ensure runtime context
    ctx.runtime_ctx_needed();

    switch (ctx.params.isEnabled(feature)) {
    case teng_feature::disabled:
        return Result_t(false);
    case teng_feature::enabled:
        return Result_t(true);
    case teng_feature::unknown:
        return failed(ctx, "isenabled", "Unknown feature '" + feature + "'");
    }
    throw std::runtime_error(__PRETTY_FUNCTION__);
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
    if (!arg.is_string_like())
        return failed(ctx, "dictexist", "Arg must be a string");
    auto key = arg.string();

    return Result_t(ctx.dict.lookup(key) || ctx.params.lookup(key));
}

/** Returns value for desired key from dictionaries.
 *
 * @param args Function arguments (list of values).
 * @param ctx Teng function ctx.
 * @param result Function's result value.
 */
Result_t getdict(Ctx_t &ctx, const Args_t &args) {
    if ((args.size() < 1) || (args.size() > 2))
        return wrongNumberOfArgs(ctx, "getdict", 1, 2);

    auto &arg0 = *args.rbegin();
    if (!arg0.is_string_like())
        return failed(ctx, "getdict", "First arg must be a string");
    auto key = arg0.string();

    // set result value
    if (auto *val = ctx.dict.lookup(key))
        return Result_t(*val);
    if (auto *val = ctx.params.lookup(key))
        return Result_t(*val);

    // no default given
    if (args.size() == 1)
        return Result_t();

    // read default value
    auto &arg1 = *++args.rbegin();
    if (!arg1.is_string_like())
        return failed(ctx, "getdict", "Second arg must be a string");
    auto def = arg1.string();
    return Result_t(def);
}

} // namespace builtin
} // namespace Teng

#endif /* TENGFUNCTIONOTHER_H */

