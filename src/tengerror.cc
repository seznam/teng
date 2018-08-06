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
 * $Id: tengerror.h,v 1.6 2011-01-19 06:39:45 burlog Exp $
 *
 * DESCRIPTION
 * Teng error handling class.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-22  (vasek)
 *             Created.
 * 2006-06-21  (sten__)
 *             Removed error duplicities.
 */

#include <string>
#include <vector>
#include <sstream>
#include <iostream>

#include "tengerror.h"

namespace Teng {

/** @short Composes log line.
  * @return composed log line */
std::string Error_t::Entry_t::getLogLine() const {
    std::ostringstream out;
    dump(out);
    return out.str();
}

void Error_t::Entry_t::dump(std::ostream &out) const {
    static const char *levelString[] = {"Debug", "Warning", "Error", "Fatal"};
    out << pos << " " << levelString[level] << ": " << msg << std::endl;
}

/** @short Dumps log into stream.
  * @param out output stream */
void Error_t::dump(std::ostream &out) const {
    for (auto &entry: entries)
        out << entry;
}

} // namespace Teng

