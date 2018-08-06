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
 * $Id: tenginstruction.h,v 1.6 2010-06-11 07:46:26 burlog Exp $
 *
 * DESCRIPTION
 * Teng identifier for teng processor.
 *
 * AUTHORS
 * Stepan Skrob <stepan@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-20  (stepan)
 *             Created.
 * 2018-07-07  (burlog)
 *             Extracted from tenginstruction.cc.
 */

#ifndef TENGIDENTIFIER_H
#define TENGIDENTIFIER_H

#include <stdio.h>
#include <string>
#include <vector>
#include <iosfwd>

#include "tengparservalue.h"

namespace Teng {

struct IdentifierPath_t {
    IdentifierPath_t(std::string value = {})
        : name(std::move(value)), path{name}
    {}
    // TODO(burlog): path jako vector view do name?
    std::string name;
    std::vector<std::string> path;
};

/** Holder of identifier name and its context.
 */
struct Identifier_t {
    /** 
     */
    explicit operator bool() const {return true;}

    /** Name of identifier.
     */
    std::string name;

    /** Context of identifier.
     *
     * For fragments and variable means how many context we must go
     * from the root one. 0 means the root, 1 one below the root etc.
     *
     * When opening new fragment 1 means open new context, 0 mean
     * continue in then current context.
     */
    uint16_t context;

    /** Depth in associated context.
     *
     * Indicates how deep the variable/fragment is in the context --
     * distance form the root.
     */
    uint16_t depth;
};

} // namespace Teng

#endif // TENGIDENTIFIER_H

