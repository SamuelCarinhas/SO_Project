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
#include "../../tools/shared/memory.h"
#include "../../config/config.h"
#include "../../utils/pipes/pipes.h"
#include "../../utils/string/string.h"

extern void init_mutex_procc();

#endif