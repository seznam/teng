#include "tengplatform.h"
#include "tengudf.h"

#include <map>

using namespace std;
using namespace Teng;

#ifndef NO_UDF_LOCKS
#include <pthread.h>
pthread_rwlock_t udfLock = PTHREAD_RWLOCK_INITIALIZER;
#endif

#pragma GCC visibility push(hidden)
map<string, UDFCallback_t> userDefinedFunction;
#pragma GCC visibility pop

void Teng::registerUDF(const string &name, Teng::UDFCallback_t udf) {

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

Teng::UDFCallback_t Teng::findUDF(const string &name) {
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
