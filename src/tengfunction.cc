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
 * $Id: tengfunction.cc,v 1.18 2008-11-20 23:32:29 burlog Exp $
 *
 * DESCRIPTION
 * Teng processor function (like len, substr, round or date)
 *
 * AUTHORS
 * Jan Nemec <jan.nemec@firma.seznam.cz>
 * Stepan Skrob <stepan@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-26  (honza)
 *             Created.
 * 2004-05-08  (vasek)
 *             Polished code.
 * 2005-06-21  (roman)
 *             Win32 support.
 * 2018-06-14  (burlog)
 *             Polished.
 */

#include <string>

#include "tenghex.h"
#include "tengutf8.h"
#include "tengplatform.h"
#include "tengfunctionutil.h"
#include "tengfunctionstring.h"
#include "tengfunctionnumber.h"
#include "tengfunctionescaping.h"
#include "tengfunctionother.h"
#include "tengfunctiondate.h"
#include "tengfunction.h"

namespace Teng {
namespace {

struct FunctionStub_t {
    const char *name;  // teng name
    bool eval;         // use for preevaluation (false for rand(), time() etc)
    Function_t func;   // C++ function addr
};

FunctionStub_t builtin_functions[] = {
     // number
    {"int", true, builtin::toint},               // like (int) in C
    {"isnumber", true, builtin::isnumber},       // true if arg is number
    {"random", false, builtin::random},          // random integer
    {"round", true, builtin::round},             // round(number, precision)
    {"numformat", true, builtin::numformat},     // format number for display

    // string
    {"len", false, builtin::len},                // like strlen in C
    {"strtolower", true, builtin::strtolower},   // utf-8 lowercase
    {"strtoupper", true, builtin::strtoupper},   // utf-8 uppercase
    {"substr", false, builtin::substr},          // like str[a:b] in Python
    {"wordsubstr", false, builtin::wordsubstr},  // substr preserving words
    {"reorder", true, builtin::reorder},         // like sprintf with %s
    {"nl2br", true, builtin::nl2br},             // convert '\n' => <br />
    {"replace", true, builtin::replace},         // string replace
    {"regex_replace", true, builtin::regex_replace}, // regex replace

    // escaping
    {"escape", false, builtin::escape},          // for example "<" => "&lt;"
    {"unescape", false, builtin::unescape},      // for example "&lt;" => "<"
    {"urlescape", true, builtin::urlescape},     // escape strange chars in urls
    {"urlunescape", true, builtin::urlunescape}, // unescape url encoding
    {"quoteescape", true, builtin::quoteescape}, // escape strange chars

    // date
    {"date", true, builtin::date},               // like strftime
    {"now", false, builtin::now},                // like gettimeofday
    {"sectotime", true, builtin::sectotime},     // convert seconds to HH:MM:SS
    {"timestamp", true, builtin::timestamp},     // returns current timestamp

    // other
    {"isenabled", true, builtin::isenabled},     // isenabled(feature)
    {"dictexist", true, builtin::dictexist},     // dictexist(key)
    {"getdict", true, builtin::getdict},         // getdict(key, default)

    // sentinel
    {nullptr, false, nullptr}                    // end of list
};

} // namespace

Invoker_t<Function_t>
findFunction(const std::string &name, bool normalRun) {
    for (auto *p = builtin_functions; p->name; ++p)
        if ((normalRun || p->eval) && (p->name == name))
            return {name, p->func};
    return {name, nullptr};
}

} // namespace Teng

