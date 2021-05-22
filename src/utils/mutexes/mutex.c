/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/
#include "mutex.h"

/**
 * @brief Initialize the given mutex to be use between processes
 * 
 * @param mutex Pointer to the mutex to initialize
 */
void init_mutex_proc(pthread_mutex_t * mutex) {
    pthread_mutexattr_t attrmutex;
    pthread_mutexattr_init(&attrmutex);
    pthread_mutexattr_setpshared(&attrmutex, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex, &attrmutex);
}

/**
 * @brief Initialize the given condition variable to be use between processes
 * 
 * @param mutex Pointer to the condition variable to initialize
 */
void init_cond_proc(pthread_cond_t * cond) {
    pthread_condattr_t attrcondv;
    pthread_condattr_init(&attrcondv);
    pthread_condattr_setpshared(&attrcondv, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(cond, &attrcondv);
}

/**
 * @brief Destroy the given mutex
 * 
 * @param mutex Pointer to the mutex to be destroyed
 */
void destroy_mutex_proc(pthread_mutex_t * mutex) {
    pthread_mutex_destroy(mutex);
}

/**
 * @brief Destroy the given condition variable
 * 
 * @param mutex Pointer to the condition variable to be destroyed
 */
void destroy_cond_proc(pthread_cond_t * cond) {
    pthread_cond_destroy(cond);
}