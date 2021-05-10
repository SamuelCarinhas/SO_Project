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
#include "../../config/config.h"

typedef struct shared_memory shared_memory_t;
typedef struct team team_t;
typedef struct car car_t;

enum box_status {
    OPEN, RESERVED, OCCUPIED
};

enum car_status {
    RACE, SAFE_MODE, BOX, GAVE_UP, FINISHED
};

struct shared_memory {
    int num_teams;
    int race_started;
    int restarting_race;
    int message_queue;
    int total_cars;
    int finish_cars;
    int end_race;
    pthread_mutex_t mutex, mutex_reset;
    pthread_cond_t new_command, reset_race;
};

struct team {
    char name[MAX_STRING];
    int num_cars;
    int res;
    int pos_array;
    int safe_cars;
    pthread_mutex_t team_mutex;
    enum box_status box;
};

struct car {
    int number, speed, reliability,
        total_malfunctions, total_refuels, total_boxstops;
    double consumption, fuel, distance, current_speed;
    team_t * team;
    pthread_t thread;
    enum car_status status;
};

extern void wait_for_start(shared_memory_t * shared_memory, pthread_mutex_t* mutex);
extern team_t * get_teams(shared_memory_t * shared_memory);
extern car_t * get_cars(shared_memory_t * shared_memory, config_t * config);
extern car_t * get_car(shared_memory_t * shared_memory, config_t * config, int pos_team, int pos_car);
extern void init_car(car_t * car, config_t * config);
extern void init_team(team_t * team);
extern void init_memory(shared_memory_t * shared_memory);
extern void show_statistics(shared_memory_t * shared_memory, config_t * config_t);

#endif