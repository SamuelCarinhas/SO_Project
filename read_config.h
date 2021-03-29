/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#ifndef READ_CONFIG_HEADER
#define READ_CONFIG_HEADER

#include <stdio.h>
#include <stdlib.h>
#include "constants.h"

#define MIN_TEAMS 3

typedef struct config_t {

    int time_units_per_second;
    int lap_distance;
    int laps;
    int teams; // Min = 3
    int max_cars_per_team;
    int failure_time_units;
    int min_repair_time; // unidades de tempo
    int max_repair_time; // unidades de tempo
    int fuel_capacity;

} config_t;

config_t * load_config();

#endif