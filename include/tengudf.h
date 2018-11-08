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
 *
 * DESCRIPTION
 * User defined functions.
 *
 * AUTHORS
 * Jan Nemec <jan.nemec@firma.seznam.cz>
 * Vaclav Blazek <blazek@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-26  (jan)
 *             Created.
 * 2018-06-07  (burlog)
 *             Rewrite to C++.
 */

#ifndef TENGUDF_H
#define TENGUDF_H

#include <vector>
#include <string>
#include <functional>

#include <tengconfig.h>
#include <tenginvoke.h>

namespace Teng {
namespace udf {

// List of values are udf arguments.
using Args_t = FunctionArgs_t;
using Result_t = FunctionResult_t;

// Type for user defined functions.
using Function_t = std::function<Result_t(const Args_t &)>;

/**
 * @short Registers user-defined function.
 * @param name name of the function (without udf.prefix)
 * @param udf user-defined callable object
 */
void registerFunction(const std::string &name, Function_t udf);

/**
 * @short finds function in global UDF list, returns pointer or 0
 * @param name name of the function (with udf. prefix)
 */
Invoker_t<Function_t> findFunction(const std::string &name);

} // namespace udf
} // namespace Teng

#endif /* TENGUDF_H */

