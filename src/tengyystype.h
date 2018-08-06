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
 * $Id$
 *
 * DESCRIPTION
 * Teng syntax analyzer.
 *
 * AUTHORS
 * Filip Volejnik <filip.volejnik@firma.seznam.cz>
 *
 * HISTORY
 * 2012-04-16  (volejnik)
 *             First version.
 */

#ifndef TENGYYSTYPE_H
#define TENGYYSTYPE_H

#include <map>
#include <vector>
#include <string>

#include "tengparservalue.h"
#include "tengerror.h"

namespace Teng {

// define left-value type
struct LeftValue_t {

    // element's left-value
    ParserValue_t val;

    // define option list type
    typedef std::map<std::string, std::string> OptionList_t;
    // teng-directive options used for building code
    OptionList_t opt;

    // define list of addresses info
    typedef std::vector<long> AddressList_t;
    // program address--tmp just for building code
    AddressList_t addr;

    // define identifier type
    typedef std::vector<std::string> Identifier_t;
    // variable identifier
    Identifier_t id;

    // position in input stream (just for lexical elements)
    // in other cases is pos-value irrelevant
    Error_t::Position_t pos;

    // program size in the time when the element was read (and then shifted)
    // this value is used for discarding parts of program in case of error
    unsigned int prgsize;
};

} // namespace Teng

#endif // TENGYYSTYPE_H
