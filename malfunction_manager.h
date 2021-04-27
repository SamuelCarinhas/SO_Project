/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#ifndef MALFUNCTION_MANAGER_HEADER
#define MALFUNCTION_MANAGER_HEADER

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
#include "read_config.h"
#include "global.h"
#include "functions.h"

void malfunction_manager(shared_memory_t *, config_t *);

#endif