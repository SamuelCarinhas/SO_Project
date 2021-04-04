/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#ifndef RACE_MANAGER_HEADER
#define RACE_MANAGER_HEADER

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include "functions.h"
#include "read_config.h"
#include "team_manager.h"
#include "race_simulator.h"
#include "global.h"

void race_manager(shared_memory_t *);

#endif