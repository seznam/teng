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
 * $Id: tengprogram.cc,v 1.1 2004-07-28 11:36:55 solamyl Exp $
 *
 * DESCRIPTION
 * Teng program is sequence of teng instructions.
 *
 * AUTHORS
 * Stepan Skrob <stepan@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-24  (stepan)
 *             Created.
 */

#include <stdio.h>

#include "tengprogram.h"

using namespace std;

using namespace Teng;

/** Print whole program into file stream.
  * @param fp File stream for output. */
void Program_t::dump(FILE *fp) const
{
    Program_t::const_iterator i;
    for (i = begin(); i != end(); ++i) {
        fprintf(fp, "%d\t", i - begin());
        (*i).dump(fp);
    }
}
