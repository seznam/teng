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
 * $Id: tengutil.cc,v 1.5 2010-06-11 08:25:35 burlog Exp $
 *
 * DESCRIPTION
 * Teng utilities.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-24  (vasek)
 *             Created.
 * 2005-06-21  (roman)
 *             Win32 support.
 *
 */

#include <vector>
#include <cstring>
#include <algorithm>

#include "platform.h"
#include "util.h"

namespace Teng {

void normalizeFilename(std::string &filename) {
    // check for empty filename
    if (filename.empty()) return;

    CONVERTNAMEBYPLATFORM(filename)

    // cache of filename parts
    std::vector<std::string> parts;
    parts.reserve(10);
    // run through filename and split it by slashes
    for (std::string::size_type slash = 0; ; ) {
        // find slash
        std::string::size_type nextSlash = filename.find('/', slash);
        // cut filename part
        std::string part = filename.substr(slash, nextSlash - slash);
        // if part is non-empty ("" or ".")
        if (!part.empty() && (part != ".")) {
            // if part means 'parent dir'
            if (part == "..") {
                // if not at root => remove previous part
                if (!parts.empty())
                    parts.pop_back();
            } else {
                // push part into cache
                parts.push_back(part);
            }
        }
        // do until end-of-string
        if (nextSlash == std::string::npos) break;
        // move after slash
        slash = nextSlash + 1;
    }

    // erase current filename
    filename.erase();
    // run through part cache and glue them with '/' together
    for (auto &part: parts) {
#ifdef WIN32
        if (!filename.empty())
#endif //WIN32
        filename.push_back('/');
        filename.append(part);
    }
}

std::string clip(std::string str, unsigned int len) {
    static const unsigned int CLIP_DOTS_COUNT = 3;
    if (str.size() <= len) return str;

    // len is even shorter than CLIP_DOTS_COUNT
    if (len <= CLIP_DOTS_COUNT) {
        str.resize(0);
        for (auto i = 0u; i < len; ++i)
            str.push_back('.');
        return str;
    }

    // str.size() > len && len > CLIP_DOTS_COUNT
    // index to the first character over the limit
    auto i = len - CLIP_DOTS_COUNT;
    switch (str[i] & 0b11000000) {
    case 0b00000000: // ascii char
    case 0b01000000: // ascii char
        str.resize(i);
        break;
    case 0b10000000: // utf-8 continuation char
        do {
            if ((str[--i] & 0b11000000) != 0b10000000)
                break;
        } while (i > 0);
        str.resize(i);
        break;
    case 0b11000000: // utf-8 leading char
        str.resize(i - 1);
        break;
    }

    // append three dots
    for (auto i = 0u; i < CLIP_DOTS_COUNT; ++i)
        str.push_back('.');
    return str;
}

std::string tolower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

std::string strerr(int errno_value) {
    char system_error[1024];
    system_error[0] = '\0';
    return strerror_r(errno_value, system_error, sizeof(system_error));
}

} // namespace Teng

