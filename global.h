/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#ifndef CONSTANTS_HEADER
#define CONSTANTS_HEADER

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAX_STRING 100
#define MIN_TEAMS 3
#define DEBUG

typedef struct config_t {

    int time_units_per_second;
    int lap_distance;
    int laps;
    int teams; // Min = 3
    int max_cars_per_team;
    int malfunction_time_units;
    int min_repair_time; // unidades de tempo
    int max_repair_time; // unidades de tempo
    int fuel_capacity;

} config_t;

enum box_status {
    OPEN, RESERVED, OCCUPIED
};

enum car_status {
    NORMAL, SAFE_MODE
};

typedef struct {
    char name[MAX_STRING];
    int num_cars, res;
    int pos_array;
} team_t;

typedef struct {
    int number, speed, reliability;
    double consuption;
    team_t * team;
    pthread_t thread;
    enum car_status status;
} car_t;

typedef struct {
    int num_teams;
    int race_started;
    pthread_mutex_t mutex;
    pthread_cond_t new_command;
} shared_memory_t;

#endif