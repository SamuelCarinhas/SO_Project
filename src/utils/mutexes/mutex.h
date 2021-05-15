#ifndef MUTEX_HEADER
#define MUTEX_HEADER

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

extern void init_mutex_proc(pthread_mutex_t * mutex);
extern void init_cond_proc(pthread_cond_t * cond);

extern void destroy_mutex_proc(pthread_mutex_t * mutex);
extern void destroy_cond_proc(pthread_cond_t * cond)

#endif