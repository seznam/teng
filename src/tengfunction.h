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
 * $Id: tengfunction.h,v 1.2 2004-12-30 12:42:01 vasek Exp $
 *
 * DESCRIPTION
 * Teng processor funcction (like len, round or formatDate)
 *
 * AUTHORS
 * Jan Nemec <jan.nemec@firma.seznam.cz>
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-26  (jan)
 *             Created.
 */

#ifndef TENGFUNCTION_H
#define TENGFUNCTION_H

#include <vector>

#include "tengparservalue.h"
#include "tengprocessor.h"

using namespace std;

namespace Teng {

/** Function in Teng (len, round, formatdate, ...)
 *         return  0 OK
 *                -1 wrong argument count
 *        other (-2) other error
 *
 * vector            argument list
 * TengParserValue_t return type 
 * */
typedef int (*Function_t)(const vector<ParserValue_t> &,
                          const Processor_t::FunctionParam_t&,
                          ParserValue_t &);


/**
 * @short finds function in global list, returns pointer or 0
 * @param name name of the function
 * @param normalRun true for normal run, false for preevaluation constat expr
 */
Function_t tengFindFunction(const string &name, bool normalRun = true);

} // namespace Teng

#endif // TENGFUNCTION_H

