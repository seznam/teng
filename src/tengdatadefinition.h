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
 * $Id: tengdatadefinition.h,v 1.1 2004-07-28 11:36:55 solamyl Exp $
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

#ifndef _TENGDATADEFINITION_H
#define _TENGDATADEFINITION_H

#include <string>
#include <vector>
#include <stdio.h>

#include "tengdictionary.h"

using namespace std;

namespace Teng {

/** @short Data definition -- dictionary of valid variables and
 *         fragments.
 *  Used for checking of data validity and documentation.
 */
class DataDefinition_t: public Dictionary_t {
public:
    /** @short Create new data definition.
     *  @param root root of relative paths
     */
    DataDefinition_t(const string &root = string())
        : Dictionary_t(root)
    {};

    /** @short Destroy data definition.
     */
    virtual ~DataDefinition_t() {};

protected:
    /** @short Parses line beginning with identifier.
     *  @param line parsed line
     *  @param name name of identifier
     *  @param value value of ifentifier
     *  @param pos position in current file
     *  @return 0 OK !0 error
     */
    virtual int parseIdentLine(const string &line, string &name,
                               string &value, Error_t::Position_t &pos);
};

} // namespace Teng

#endif // _TENGDATADEFINITION_H
