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
map<string, UDF_t*> userDefinedFunction;
#pragma GCC visibility pop

UDF_t* Teng::tengRegisterUDF(const string &name, UDF_t *udf) {
UDF_t *prev;
std::string qId = "udf." + name;
#ifndef NO_UDF_LOCKS
    pthread_rwlock_wrlock(&udfLock);
#endif

    prev = userDefinedFunction[qId];
    userDefinedFunction[qId] = udf;

#ifndef NO_UDF_LOCKS
    pthread_rwlock_unlock(&udfLock);
#endif

    return prev;
}

UDF_t *Teng::tengFindUDF(const string &name) {
UDF_t *res;

#ifndef NO_UDF_LOCKS
    pthread_rwlock_rdlock(&udfLock);
#endif

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
                for (map<string, UDF_t*>::iterator it = userDefinedFunction.begin();
                    it != userDefinedFunction.end(); it++) {
                    delete it->second;
                }
                userDefinedFunction.clear();
            #ifndef NO_UDF_LOCKS
                pthread_rwlock_unlock(&udfLock);
            #endif
        }
};

static UDFCleanup_t __cleanupUDF;
#pragma GCC visibility pop
