/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#ifndef MEMORY_HEADER
#define MEMORY_HEADER

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
#include "../../utils/mutexes/mutex.h"
#include "../../utils/log/log.h"
#include "../../config/config.h"
#include "../../utils/pipes/pipes.h"
#include "../../utils/string/string.h"

typedef struct shared_memory shared_memory_t;
typedef struct team team_t;
typedef struct car car_t;

enum box_status {
    OPEN, RESERVED, OCCUPIED
};

enum car_status {
    RACE, SAFE_MODE, BOX, GAVE_UP, FINISHED
};

shared_memory_t {
    int num_teams;
    int race_started;
    int message_queue;
    int finish_cars;
    pthread_mutex_t mutex;
    pthread_cond_t new_command;
};

team_t {
    char name[MAX_STRING];
    int num_cars;
    int res;
    int pos_array;
    int safe_cars;
    pthread_mutex_t team_mutex;
    enum box_status box;
};

car_t {
    int number, speed, reliability,
        total_malfunctions, total_refuels, total_boxstops;
    double consuption, fuel, distance, current_speed;
    team_t * team;
    pthread_t thread;
    enum car_status status;
};

extern team_t * get_teams(shared_memory_t * shared_memory);
extern car_t * get_cars(shared_memory_t * shared_memory, config_t * config);
extern car_t * get_car(shared_memory_t * shared_memory, config_t * config, int pos_team, int pos_car);

#endif