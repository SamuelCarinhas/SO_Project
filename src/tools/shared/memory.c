/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "memory.h"

team_t * get_teams(shared_memory_t * shared_memory) {
    return (team_t *) (shared_memory + 1);
}

car_t * get_cars(shared_memory_t * shared_memory, config_t * config) {
    return (car_t *) (get_teams(shared_memory) + config->teams);
}

car_t * get_car(shared_memory_t * shared_memory, config_t * config, int pos_team, int pos_car) {
    return (car_t *) (get_cars(shared_memory, config) + pos_team * config->max_cars_per_team + pos_car);
}