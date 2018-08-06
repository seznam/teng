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

#include "tengplatform.h"
#include "tengutil.h"

namespace Teng {
namespace {

// count of "." characters, which are appended to clipped string
const unsigned int CLIP_DOTS_COUNT = 3;

} // namespace

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

void clipString(std::string &str, unsigned int len) {
    if (str.size() + CLIP_DOTS_COUNT > len) {
        str = str.substr(0, std::max((int)len - (int)CLIP_DOTS_COUNT, 0));
        // find previous correct utf8 character
        while (str.length() && (str[str.length() - 1] & 0x80) == 0x80) {
            char ch = str[str.length() - 1];
            str.erase(str.length() - 1, 1);
            // char 11xxxxxx is begin of utf8 char, we can break
            if ((ch & 0xc0) == 0xc0)
                break;
        }
        // add ... string to end of value
        for (unsigned int i = 0; i < CLIP_DOTS_COUNT && str.length() < len; i++)
            str += ".";
    }
}

std::string tolower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

std::string strerr() {
    char system_error_string[1024];
    system_error_string[0] = '\0';
    strerror_r(errno, system_error_string, sizeof(system_error_string));
    return system_error_string;
}

} // namespace Teng

