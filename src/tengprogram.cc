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
 * $Id: tengprogram.cc,v 1.2 2007-05-21 15:43:28 vasek Exp $
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

#include <cstdio>
#include <iomanip>

#include "tengfilestream.h"
#include "tengprogram.h"

namespace Teng {

void Program_t::dump(FILE *fp) const {
    FileStream_t stream(fp);
    dump(stream);
}

void Program_t::dump(std::ostream &out) const {
    for (auto &instr: instrs) {
        out << std::setw(3) << std::setfill('0') << std::noshowpos
            << std::distance(instrs.data(), &instr) << '\t';
        instr.dump(out);
    }
}

} // namespace Teng

