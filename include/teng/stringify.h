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
 * $Id: tengvalue.h,v 1.3 2007-05-21 15:43:28 vasek Exp $
 *
 * DESCRIPTION
 * Teng number to string.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-06-07  (burlog)
 *             Created
 */

#ifndef TENGSTRINGIFY_H
#define TENGSTRINGIFY_H

#include <cstdio>
#include <string>
#include <cfloat>

#include <teng/stringview.h>
#include <teng/types.h>

namespace Teng {

/** Converts the integral number to string.
 */
template <typename writer_t, typename... args_t>
auto stringify(IntType_t value, writer_t &&writer, args_t &&...args) {
    // produce at least d
    char buffer[24];
    auto len = snprintf(buffer, sizeof(buffer), "%jd", value);
    return writer(string_view_t(buffer, len), std::forward<args_t>(args)...);
}

/** Converts the real number to string.
 */
template <typename writer_t, typename... args_t>
auto stringify(double value, writer_t &&writer, args_t &&...args) {
    // produce at least d.d
    char buffer[3 + DBL_MANT_DIG - DBL_MIN_EXP];
    auto len = snprintf(buffer, sizeof(buffer), "%#f", value);

    // remove trailing zeroes
    for (; len > 2; --len) {
        if (buffer[len - 1] == '0')
            if (buffer[len - 2] != '.')
                continue;
        break;
    }

    // done
    return writer(string_view_t(buffer, len), std::forward<args_t>(args)...);
}

/** Converts the number to string.
 */
template <typename type_t>
inline std::string stringify(type_t value) {
    return stringify(value, [] (const string_view_t &v) {return v.str();});
}

} // namespace Teng

#endif /* TENGSTRINGIFY_H */

