/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "memory.h"

void show_statistics(shared_memory_t * shared_memory, config_t * config) {
    write_log("\n");
    pthread_mutex_lock(&shared_memory->mutex); //!!!!!!!!!!!!!!!!!!!!! USAR MUTEX DAS EQUIPAS !!!!!!!!!!!!!!!!!!!!!!!!!!
    int n_cars = 0;
    team_t * teams = get_teams(shared_memory);
    for(int i = 0; i < shared_memory->num_teams; i++) {
        n_cars += teams[i].num_cars;
    }
    int max_statistics = (n_cars >= TOP_STATISTICS) ? TOP_STATISTICS : n_cars;
    car_t best_cars[max_statistics];
    for(int i = 0; i < max_statistics; i++) best_cars[i].distance = -1;
    car_t * car;
    for(int i = 0; i < shared_memory->num_teams; i++) {
        for(int j = 0; j < teams[i].num_cars; j++) {
            car = get_car(shared_memory, config, i, j);
            int l;
            for(l = 0; l < max_statistics && car->distance < best_cars[l].distance; l++);
            if(l == max_statistics) continue;
            for(int k = max_statistics - 1; k > l; k--) best_cars[k] = best_cars[k-1];
            best_cars[l] = *car;
        }
    }
    pthread_mutex_unlock(&shared_memory->mutex);

    write_log("STATISTICS:\n");
    write_log("| RANK | CAR | TEAM | LAPS | STOPS | FUEL |\n");
    for(int i = 0; i< max_statistics; i++){
        write_log("| %4d | %3d | %4s | %4d | %5d | %4.1f |\n", (i+1), best_cars[i].number, best_cars[i].team->name, (int)(best_cars[i].distance/config->lap_distance), best_cars[i].total_boxstops, best_cars[i].fuel);
    }
}

void init_car(car_t * car, config_t * config) {
    car->status = RACE;
    car->fuel = config->fuel_capacity;
    car->distance = 0;
    car->total_malfunctions = 0;
    car->total_refuels = 0;
    car->total_boxstops = 0;
}

void init_team(team_t * team) {
    team->safe_cars = 0;
}

void init_memory(shared_memory_t * shared_memory) {
    shared_memory->finish_cars = 0;
}

void wait_for_start(shared_memory_t * shared_memory, pthread_mutex_t * mutex) {
    pthread_mutex_lock(mutex);
    while (shared_memory->race_started == 0)
        pthread_cond_wait(&shared_memory->new_command, mutex);
    pthread_mutex_unlock(mutex);
}

team_t * get_teams(shared_memory_t * shared_memory) {
    return (team_t *) (shared_memory + 1);
}

car_t * get_cars(shared_memory_t * shared_memory, config_t * config) {
    return (car_t *) (get_teams(shared_memory) + config->teams);
}

car_t * get_car(shared_memory_t * shared_memory, config_t * config, int pos_team, int pos_car) {
    return (car_t *) (get_cars(shared_memory, config) + pos_team * config->max_cars_per_team + pos_car);
}