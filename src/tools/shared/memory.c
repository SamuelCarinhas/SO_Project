/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "memory.h"

char * car_string[] = {
    "RACE",
    "SAFE_MODE",
    "BOX",
    "GAVE_UP",
    "FINISHED"
};

void show_statistics(shared_memory_t * shared_memory, config_t * config) {
    write_log("\n");
    pthread_mutex_lock(&shared_memory->mutex);
    int num_teams = shared_memory->num_teams;
    int total_cars = shared_memory->total_cars;
    pthread_mutex_unlock(&shared_memory->mutex);

    team_t * teams = get_teams(shared_memory);
    for(int i = 0; i < num_teams; i++)
        pthread_mutex_lock(&teams[i].team_mutex);
    
    int max_statistics = (total_cars >= TOP_STATISTICS) ? TOP_STATISTICS : total_cars;
    car_t car_ranking[total_cars];
    for(int i = 0; i < total_cars; i++)
        car_ranking[i].distance = -1;

    car_t * car;
    int total_malfunctions = 0, total_refuels = 0;
    for(int i = 0; i < num_teams; i++) {
        for(int j = 0; j < teams[i].num_cars; j++) {
            car = get_car(shared_memory, config, i, j);
            int l;
            total_malfunctions += car->total_malfunctions;
            total_refuels +=car->total_refuels;
            for(l = 0; l < total_cars && car->distance < car_ranking[l].distance; l++);
            if(l == total_cars) continue;
            for(int k = total_cars - 1; k > l; k--)
                car_ranking[k] = car_ranking[k-1];
            car_ranking[l] = *car;
        }
    }
    
    for(int i = 0; i < num_teams; i++)
        pthread_mutex_unlock(&get_teams(shared_memory)[i].team_mutex);
 
    char buffer[MAX_STRING*2000] = "STATISTICS:\n";
    char buffer2[MAX_STRING*100];

    
    snprintf(buffer2, MAX_STRING*2000, "| RANK | CAR | TEAM | LAPS | STOPS | FUEL |   STATUS   |\n");
    strcat(buffer, buffer2);
    
    for(int i = 0; i < max_statistics; i++) {
        snprintf(buffer2, MAX_STRING*100, "| %4d | %3d | %4s | %4d | %5d | %4.1f | %10s | \n", (i+1), car_ranking[i].number, car_ranking[i].team->name, (int)(car_ranking[i].distance/config->lap_distance), car_ranking[i].total_boxstops, car_ranking[i].fuel, car_string[car_ranking[i].status]);
        strcat(buffer, buffer2);
    }
    snprintf(buffer2, MAX_STRING*100, "--------------------------------------------------------\n");
    strcat(buffer, buffer2);
    snprintf(buffer2, MAX_STRING*100, "| %4d | %3d | %4s | %4d | %5d | %4.1f | %10s | \n", (total_cars), car_ranking[total_cars -1].number, car_ranking[total_cars -1].team->name, (int)(car_ranking[total_cars -1].distance/config->lap_distance), car_ranking[total_cars -1].total_boxstops, car_ranking[total_cars -1].fuel, car_string[car_ranking[total_cars -1].status]);
    strcat(buffer, buffer2);

    snprintf(buffer2, MAX_STRING*100, "--------------------------------------------------------\nTOTAL MALFUNCTIONS: %d\nTOTAL REFUELS: %d\n", total_malfunctions, total_refuels);
    strcat(buffer, buffer2);

    write_log(buffer);
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
    shared_memory->race_started = 0;
    shared_memory->end_race = 0;
    shared_memory->restarting_race = 0;
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