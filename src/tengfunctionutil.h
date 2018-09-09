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

/** Handy struct that caches converted numeric arguments to string.
 */
struct string_ptr_t {
    /** Printer for Value_t::print() that return pointer to given const
     * std::string & or pointer to cached string.
     */
    struct cache_t {
        const std::string *operator()(const string_view_t &v) {
            tmp.assign(v.data(), v.size());
            return &tmp;
        }
        const std::string *operator()(const std::string &str) {
            return &str;
        }
        std::string tmp; //!< storage of string repr of non string values
    };

    /** C'tor.
     */
    string_ptr_t(const Value_t &value)
        : arg(value.print(cached_str))
    {}

    // don't copy
    string_ptr_t(const string_ptr_t &) = delete;
    string_ptr_t &operator=(const string_ptr_t &) = delete;

    // ptr access
    const std::string *operator->() const {return arg;}
    const std::string &operator*() const {return *arg;}
    char operator[](std::string::size_type i) const {return (*arg)[i];}

    cache_t cached_str;     //!< where are non string values cached
    const std::string *arg; //!< pointer to string value or to tmp
};

/** Convenient function for reporting error.
 */
Result_t failed(Ctx_t &ctx, const char *fun, const std::string &msg) {
    // TODO(burlog): pos?!
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

