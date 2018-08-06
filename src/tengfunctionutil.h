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
 *             Moved from tengfunction.cc and polished.
 */

#ifndef TENGFUNCTIONUTIL_H
#define TENGFUNCTIONUTIL_H

#include <string>

#include <tenglogging.h>
#include <tengfunction.h>

namespace Teng {
namespace builtin {

// shortcuts
using Ctx_t = FunctionCtx_t;
using Args_t = FunctionArgs_t;
using Result_t = FunctionResult_t;

/** Handy struct that cache converted numeric arguments to string.
 */
struct str {
    str(const Parser::Value_t &value): arg(&value.str(tmp)) {}
    operator const std::string &() const {return *arg;}
    const std::string *operator->() const {return arg;}
    const std::string &operator*() const {return *arg;}
    char operator[](std::string::size_type i) const {return (*arg)[i];}
    std::string tmp;
    const std::string *arg;
};

/** Convenient function for reporting error.
 */
Result_t failed(Ctx_t &ctx, const char *fun, const std::string &msg) {
    logError(ctx.err, {}, std::string(fun) + "(): " + msg);
    return Result_t();
}

/** Convenient function for reporting error.
 */
Result_t wrongNumberOfArgs(Ctx_t &ctx, const char *fun, int exp) {
    return failed(
        ctx,
        fun,
        "the function expects exactly " + std::to_string(exp) + " arg(s)"
    );
}

/** Convenient function for reporting error.
 */
Result_t wrongNumberOfArgs(Ctx_t &ctx, const char *fun, int f, int t) {
    return failed(
        ctx,
        fun,
        "the function expects from " + std::to_string(f)
        + " to " + std::to_string(t) + " args"
    );
}

/** Convenient function for reporting error.
 */
Result_t atLeastXArg(Ctx_t &ctx, const char *fun, int x) {
    return failed(
        ctx,
        fun,
        "the function expects at least " + std::to_string(x) + " arg(s)"
    );
}

} // namespace builtin
} // namespace Teng

#endif /* TENGFUNCTIONUTIL_H */

