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
 * Teng processor function (like len, round or formatDate)
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

#include <mutex>
#include <unordered_map>

#include "platform.h"
#include "teng/udf.h"

namespace Teng {
namespace udf {
namespace {

/** Functions registry.
 */
class Registry_t {
public:
    /** Returns registered function for given name if any or empty function.
     */
    Function_t find(const std::string &name) {
#ifndef NO_UDF_LOCKS
        std::lock_guard<std::mutex> locked(mutex);
#endif /* NO_UDF_LOCKS */
        auto ifunction = registry.find(name);
        return ifunction == registry.end()
             ? Function_t{}
             : ifunction->second;
    }

    /** Inserts new value to registry.
     */
    void insert(const std::string &name, Function_t function) {
#ifndef NO_UDF_LOCKS
        std::lock_guard<std::mutex> locked(mutex);
#endif /* NO_UDF_LOCKS */
        registry.emplace("udf." + name, std::move(function));
    }

    /** D'tor
     */
    ~Registry_t() {
#ifndef NO_UDF_LOCKS
        std::lock_guard<std::mutex> locked(mutex);
#endif /* NO_UDF_LOCKS */
        registry.clear();
    }

    std::mutex mutex;
    std::unordered_map<std::string, Function_t> registry;
} registered_functions;

} // namespace

void registerFunction(const std::string &name, Function_t function) {
    registered_functions.insert(name, std::move(function));
}

Invoker_t<Function_t> findFunction(const std::string &name) {
    return {name, registered_functions.find(name)};
}

} // namespace udf
} // namespace Teng

