/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/
#ifndef CAR_THREAD_HEADER
#define CAR_THREAD_HEADER

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
#include "../../race_simulator.h"
#include "../../../utils/mutexes/mutex.h"
#include "../../../utils/log/log.h"
#include "../../../tools/messages/message.h"
#include "../../../tools/shared/memory.h"
#include "../../../config/config.h"
#include "../../../utils/pipes/pipes.h"
#include "../../../utils/string/string.h"
#include "../../../utils/clock/clock.h"
#include "box_thread.h"

typedef struct car_args car_args_t;

struct car_args {
    shared_memory_t * shared_memory;
    config_t * config;
    team_t * team;
    car_t * car;
    int fd;
};

extern void * car_thread(void * args);
extern car_args_t * create_car_args(shared_memory_t * shared_memory, config_t * config, team_t * team, car_t * car, int fd);

#endif