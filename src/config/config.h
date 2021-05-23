/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#ifndef CONFIG_HEADER
#define CONFIG_HEADER

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
#include "../utils/log/log.h"
#include "../utils/numeric/numeric.h"
#include "../utils/string/string.h"

// Global constant values
#define MAX_STRING 5000
#define MIN_TEAMS 3
#define PIPE_NAME "COMMAND_PIPE"
#define LOG_FILE "log/log.txt"
#define TOP_STATISTICS 5
#define MIN_TEAM_NAME 3
#define MAX_TEAM_NAME 16
#define MAX_TIME_UNITS_PER_SECOND 150

// Flag to enable debug logs
// Remove it to remove the debug prints
#define DEBUG

typedef struct config config_t;

struct config {
    int time_units_per_second;
    int lap_distance;
    int laps;
    int teams; // Min = 3
    int max_cars_per_team;
    int malfunction_time_units;
    int min_repair_time; // unidades de tempo
    int max_repair_time; // unidades de tempo
    int fuel_capacity;
};

extern config_t * load_config();

#endif