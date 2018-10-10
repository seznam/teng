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
 * $Id: tengstructs.cc,v 1.4 2007-05-21 15:43:28 vasek Exp $
 *
 * DESCRIPTION
 * Teng data types -- json utils.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-07-07  (burlog)
 *             Created.
*/

#include <string>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "tengstringview.h"
#include "tengconfig.h"

namespace Teng {
namespace json {

inline void quoted_unicode_char(std::ostream &o, char ch) {
    o << "\\u00" << std::hex << std::setw(2) << std::setfill('0') << int(ch);
}

inline void quote_string(std::ostream &o, const string_view_t &value) {
    o << '"';
    for (char ch: value) {
        switch (ch) {
        case 0 ... 8:
        case 11 ... 12:
        case 14 ... 31: quoted_unicode_char(o, ch); break;
        case '\n': o << "\\n"; break;
        case '\r': o << "\\r"; break;
        case '\t': o << "\\t"; break;
        case '\\': o << "\\\\"; break;
        case '"': o << "\\\""; break;
        default: o << ch; break;
        }
    }
    o << '"';
}

} // namespace json
} // namespace Teng

