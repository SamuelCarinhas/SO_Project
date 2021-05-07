/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#ifndef RACE_SIMULATOR_HEADER
#define RACE_SIMULATOR_HEADER

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
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
#include <errno.h>
#include "./managers/malfunction_manager.h"
#include "./managers/race_manager.h"
#include "./managers/team_manager.h"
#include "../utils/mutexes/mutex.h"
#include "../utils/log/log.h"
#include "../tools/shared/memory.h"
#include "../config/config.h"
#include "../utils/pipes/pipes.h"
#include "../utils/string/string.h"

#endif