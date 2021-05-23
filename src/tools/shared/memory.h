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

// Constant to use in case for the max car rank
#define LAST_RANK 1000000

typedef struct car car_t;
typedef struct team team_t;
typedef struct shared_memory shared_memory_t;
typedef struct box box_t;
typedef struct clock sync_clock_t;

enum box_status {
    OPEN, RESERVED, OCCUPIED
};

struct box {
    car_t * car;
    int has_car;
    int request_reservation;
    enum box_status status;
    pthread_cond_t request, car_leave;
    pthread_mutex_t mutex, join_mutex, leave_mutex;
};


enum car_status {
    RACE, SAFE_MODE, BOX, GAVE_UP, FINISHED
};

extern char * car_string[];

struct clock {
    pthread_mutex_t mutex;
    pthread_cond_t time_cond;
};

struct shared_memory {
    int ranking;
    int end_race;
    int num_teams;
    int total_cars;
    int finish_cars;
    int race_started;
    int message_queue;
    int restarting_race;
    int waiting_for_reset;
    sync_clock_t clock;
    pthread_mutex_t mutex, mutex_reset;
    pthread_cond_t new_command,  reset_race;
};

struct team {
    int res;
    int num_cars;
    int pos_array;
    int safe_cars;
    char name[MAX_STRING];
    pthread_mutex_t team_mutex;
    box_t box;
};

struct car {
    int rank;
    int speed;
    int number;
    int problem;
    int reliability;
    int total_refuels;
    int total_boxstops;
    int total_malfunctions;
    double fuel;
    double distance;
    double consumption;
    team_t * team;
    pthread_t thread;
    pthread_mutex_t car_mutex;
    enum car_status status;
};

extern int wait_for_start(shared_memory_t * shared_memory);
extern team_t * get_teams(shared_memory_t * shared_memory);
extern car_t * get_cars(shared_memory_t * shared_memory, config_t * config);
extern car_t * get_car(shared_memory_t * shared_memory, config_t * config, int pos_team, int pos_car);
extern void init_car(car_t * car, config_t * config);
extern void init_team(team_t * team);
extern void init_memory(shared_memory_t * shared_memory);
extern void show_statistics(shared_memory_t * shared_memory, config_t * config_t);

#endif