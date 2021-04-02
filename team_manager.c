/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "team_manager.h"

void * car_thread(void * p) {
    car_t car = *((car_t *) p);
    printf("Team: %s | Car: %d | Speed: %d | Consuption: %.2f | Reliability: %d\n", car.team_name, car.number, car.speed, car.consuption, car.reliability);
    pthread_exit(NULL);
    return NULL;
}

void team_manager(shared_memory_t * shared_memory, team_t * team) {
    while(1) {
        pthread_mutex_lock(&shared_memory->mutex);
        while (team->num_cars == team->res && shared_memory->race_started == 0) {
            // IS THIS CORRECT?
            pthread_cond_wait(&team->new_command, &shared_memory->mutex);
        }

        int can_start = shared_memory->race_started;

        pthread_mutex_unlock(&shared_memory->mutex);
        
        if(can_start) {
            break;
        }
        
        pthread_create(&team->cars[team->res].thread, NULL, car_thread, &team->cars[team->res]);
        team->res++;
    }

    for(int i = 0; i < team->res; i++) {
        printf("Team %s : Car %d is leaving\n", team->name, team->cars[i].number);
        pthread_join(team->cars[i].thread, NULL);
    }
}