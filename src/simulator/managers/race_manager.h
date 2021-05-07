/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#ifndef RACE_MANAGER_HEADER
#define RACE_MANAGER_HEADER

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
#include "../race_simulator.h"
#include "../../utils/mutexes/mutex.h"
#include "../../utils/log/log.h"
#include "../../tools/shared/memory.h"
#include "../../config/config.h"
#include "../../utils/pipes/pipes.h"
#include "../../utils/string/string.h"

extern void race_manager(shared_memory_t * shared_memory, config_t * config);

#endif