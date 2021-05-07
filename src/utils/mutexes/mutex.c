#include "mutex.h"

void init_mutex_procc(pthread_mutex_t * mutex) {
    pthread_mutexattr_t attrmutex;
    pthread_mutexattr_init(&attrmutex);
    pthread_mutexattr_setpshared(&attrmutex, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex, &attrmutex);
}

void init_cond_procc(pthread_cond_t * cond) {
    pthread_condattr_t attrcondv;
    pthread_condattr_init(&attrcondv);
    pthread_condattr_setpshared(&attrcondv, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(cond, &attrcondv);
}