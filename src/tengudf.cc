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

#include "tengplatform.h"
#include "tengudf.h"

#include <map>

namespace Teng {

#ifndef NO_UDF_LOCKS
#include <pthread.h>
pthread_rwlock_t udfLock = PTHREAD_RWLOCK_INITIALIZER;
#endif

#pragma GCC visibility push(hidden)
std::map<std::string, UDFCallback_t> userDefinedFunction;
#pragma GCC visibility pop

void registerUDF(const std::string &name, UDFCallback_t udf) {

std::string qId = "udf." + name;
#ifndef NO_UDF_LOCKS
    pthread_rwlock_wrlock(&udfLock);
#endif

    userDefinedFunction[qId] = udf;

#ifndef NO_UDF_LOCKS
    pthread_rwlock_unlock(&udfLock);
#endif

    return;
}

UDFCallback_t findUDF(const std::string &name) {
UDFCallback_t res;

#ifndef NO_UDF_LOCKS
    pthread_rwlock_rdlock(&udfLock);
#endif

    if ( userDefinedFunction.find(name) != userDefinedFunction.end() )
        res = userDefinedFunction[name];

#ifndef NO_UDF_LOCKS
    pthread_rwlock_unlock(&udfLock);
#endif
    return res;
}

#pragma GCC visibility push(hidden)

class UDFCleanup_t {
    protected:
    public:
        UDFCleanup_t() {
        }

        ~UDFCleanup_t() {
            #ifndef NO_UDF_LOCKS
                pthread_rwlock_wrlock(&udfLock);
            #endif
                userDefinedFunction.clear();
            #ifndef NO_UDF_LOCKS
                pthread_rwlock_unlock(&udfLock);
            #endif
        }
};

static UDFCleanup_t __cleanupUDF;
#pragma GCC visibility pop

} // namespace Teng

