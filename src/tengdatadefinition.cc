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
 * $Id: tengdatadefinition.cc,v 1.1 2004-07-28 11:36:55 solamyl Exp $
 *
 * DESCRIPTION
 * Data definition dictionary.
 *
 * AUTHORS
 * Jan Nemec <jan.nemec@firma.seznam.cz>
 *
 * HISTORY
 * 2003-10-06  (jan)
 *             Created.
 */

#include "tengdatadefinition.h"

using namespace std;

using namespace Teng;

int DataDefinition_t::parseIdentLine(const string &line, string &name,
                string &value, Error_t::Position_t &pos)
{
    int wasDot;
    // get all valid chars (assumes that first char is not number)
    string::const_iterator iline = line.begin();
    wasDot = 0;
    for (; iline != line.end(); ++iline) {
        if (!(isalnum(*iline) || (*iline == '_') || (*iline == '.')))
            break;
        if (wasDot && *iline == '.') break;
        wasDot = (*iline == '.');
        pos.advanceColumn();
    }
    if (iline != line.end() || wasDot) {
        // if first non-valid char is not white, report it as error
        if (!isspace(*iline)) {
            err.logError(Error_t::LL_ERROR, pos,
                         "Invalid character in identifier");
            return -1;
        }
    }
    // cut name
    name = line.substr(0, iline - line.begin());
    // parse rest of line as value
    return parseValueLine(line.substr(iline - line.begin()), value, pos);
}

