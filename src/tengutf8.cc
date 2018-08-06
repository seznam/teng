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

#include <memory>
#include <glib.h>
#include <pcre++.h>

#include "tengutf8.h"

namespace Teng {
namespace utf8 {
namespace {

// unique ptr type for gchar *
using g_char_ptr_t = std::unique_ptr<gchar, decltype(::free) *>;

} // namespace

std::size_t strlen(const std::string &str) {
    int chars = 0;
    for (auto istr = str.begin(), estr = str.end(); istr != estr;) {
        int bytes = 0;
        char tmp = *istr;

        while (tmp & 0x80) {
            ++bytes;
            tmp <<= 1;
        }

        if (!bytes) {
            istr++;
            chars++;
            continue;
        }

        if ((bytes == 1) || (bytes > 6)) {
            istr++;
            chars++;
            continue;
        }

        --bytes;
        auto bstr = istr + 1;
        for (; (bstr != estr) && bytes; ++bstr, --bytes) {
            if ((*bstr & 0xC0) != 0x80) break;
        }

        if (bytes) {
            while (istr != bstr) {
                istr++;
                chars++;
            }
            continue;
        }
        else {
                istr = bstr;
                chars++;
            }
    }
    return chars;
}

std::string
substr(const std::string &str, int s, int e, std::string p1, std::string p2) {
    std::string result;
    int l = strlen(str);
    if (s < 0) s = l + s;
    if (e < 0) e = l + e;

    if (s <= 0) {
        s = 0;
        p1 = "";
    }

    if (e >= l) {
        e = l;
        p2 = "";
    }

    if (!l || s >= l || e <= 0 || e <= s) {
        return p1 + p2;
    }
    else result = "";
    int i = 0;

    for (auto istr = str.begin(), estr = str.end(); istr != estr;) {
        int bytes = 0;
        char tmp = *istr;

        while (tmp & 0x80) {
            ++bytes;
            tmp <<= 1;
        }

        if (!bytes) {
            if (i >= s) result += *istr;
            istr++;
            i++;
            if (i >= e) return p1 + result + p2;
            continue;
        }
        if ((bytes == 1) || (bytes > 6)) {
            if (i >= s) result += *istr;
            istr++;
            i++;
            if (i >= e) return p1 + result + p2;
            continue;
        }
        --bytes;
        auto bstr = istr + 1;
        for (; (bstr != estr) && bytes; ++bstr, --bytes) {
            if ((*bstr & 0xC0) != 0x80) break;
        }
        if (bytes) {
            while (istr != bstr) {
                if (i >= s) result += *istr;
                istr++;
                i++;
                if (i >= e) return p1 + result + p2;
            }
        } else {
            while (istr != bstr) {
                if (i >= s) result += *istr;
                istr++;
            }
            i++;
            if (i >= e) return p1 + result + p2;
        }
    }
    return p1 + result + p2;
}

void substr(const std::string &str, int &s, int &e) {
    int l = strlen(str), index, end = str.size(),
        sset = 0, chars = 0;
    if (!l) {
    empty:
        s = 0;
        e = 1;
        return;
    }

    if (s < 0) s = l + s;
    if (e < 0) e = l + e;
    if (s < 0) s = 0;
    if (e > l) e = l;
    if (s >= l || e <= 0 || e <= s) goto empty;
    if (!s) sset = 0;
    for (index = 0; index < end; ) {
        int bytes = 0;
        char tmp = str[index];
        // compute number of bytes in char
        while (tmp & 0x80) {
            ++bytes;
            tmp <<= 1;
        }
        // ASCII char
        if (!bytes) {
            chars++;
            index++;
            if (!sset && chars == s) {
                s = index;
                sset = 1;
            }
            if (chars == e) {
                e = index;
                return;
            }
            continue;
        }
        // wrong char
        if ((bytes == 1) || (bytes > 6)) {
            chars++;
            index++;
            if (!sset && chars == s) {
                s = index;
                sset = 1;
            }
            if (chars == e) {
                e = index;
                return;
            }
            continue;
        }
        --bytes;
        int bstr = index + 1;
        for (; (bstr < end) && bytes; ++bstr, --bytes) {
            if ((str[bstr] & 0xC0) != 0x80) break;
        }
        while (index < bstr) {
            index++;
        }
        if (!bytes) {
            //    index++;
            chars++;
            if (!sset && chars == s) {
                s = index;
                sset = 1;
            }
            if (chars == e) {
                e = index;
                return;
            }
        }
    }
}

std::string tolower(const std::string &str) {
    g_char_ptr_t ptr(g_utf8_strdown(str.data(), str.size()), ::free);
    return {ptr.get()};
}

std::string toupper(const std::string &str) {
    g_char_ptr_t ptr(g_utf8_strup(str.data(), str.size()), ::free);
    return {ptr.get()};
}

std::string
regex_replace(
    const std::string &where,
    const std::string &pattern,
    const std::string &repl
) {
    return pcrepp::Pcre(pattern, PCRE_GLOBAL | PCRE_UTF8).replace(where, repl);
}

} // namespace utf8
} // namespace Teng

