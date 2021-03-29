/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#ifndef TEAM_MANAGER_HEADER
#define TEAM_MANAGER_HEADER

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
#include "read_config.h"
#include "functions.h"

enum box_status {
    OPEN, RESERVED, OCCUPIED
};

typedef struct car_t {
    int number, speed, reliability;
    double consuption;
    char team_name[MAX_STRING];
    pthread_t thread;
} car_t;

typedef struct team_t {
    char name[MAX_STRING];
    car_t * cars;
} team_t;

void team_manager(config_t *, team_t *);

#endif