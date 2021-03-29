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

void team_manager(config_t * config, team_t * team) {
    int a = config->max_cars_per_team; a--;
    strcpy(team->cars[0].team_name, team->name);
    team->cars[0].consuption = 1.02;
    team->cars[0].number = 13;
    team->cars[0].speed = 250;
    team->cars[0].reliability = 95;
    

    pthread_create(&team->cars[0].thread, NULL, car_thread, &team->cars[0]);
    
    strcpy(team->cars[1].team_name, team->name);

    team->cars[1].consuption = 0.04;
    team->cars[1].speed = 32;
    team->cars[1].reliability = 90;
    team->cars[1].number = 2;
    
    pthread_create(&team->cars[1].thread, NULL, car_thread, &team->cars[1]);

    pthread_join(team->cars[0].thread, NULL);
    pthread_join(team->cars[1].thread, NULL);
}