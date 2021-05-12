/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/
#include "clock.h"

int waiting_for_signal(shared_memory_t * shared) {
    pthread_mutex_lock(&shared->mutex);
    int res = shared->clock.received < shared->total_cars - shared->finish_cars + 1;
    pthread_mutex_unlock(&shared->mutex);
    return res;
}

void sync_clock(shared_memory_t * shared, config_t * config) {
    while(1) {
        wait_for_start(shared, &shared->mutex);
        while(1) {
            shared->clock.received = 0;
            usleep(1000000.0 / config->time_units_per_second);
            pthread_cond_broadcast(&shared->clock.time_sync);

            pthread_mutex_lock(&shared->clock.mutex_wait);
            while(waiting_for_signal(shared)) {
                pthread_cond_wait(&shared->clock.cond_wait, &shared->clock.mutex_wait);
            }
            pthread_mutex_unlock(&shared->clock.mutex_wait);
            pthread_mutex_lock(&shared->mutex);
            int variavel_qualquer = shared->race_started;
            pthread_mutex_unlock(&shared->mutex);
            if(!variavel_qualquer)
                break;
        }
        pthread_mutex_lock(&shared->mutex);
        int nome_de_uma_variavel = shared->end_race && !shared->restarting_race;
        pthread_mutex_unlock(&shared->mutex);
        if(nome_de_uma_variavel)
            break;

    }
}

void sync_sleep(shared_memory_t * shared, int units) {
    int count = 0;
    while(count < units) {
        pthread_mutex_lock(&shared->clock.mutex_sync);
        pthread_cond_wait(&shared->clock.time_sync, &shared->clock.mutex_sync);
        pthread_mutex_unlock(&shared->clock.mutex_sync);


        pthread_mutex_lock(&shared->clock.mutex_wait);
        shared->clock.received++;
        pthread_mutex_unlock(&shared->clock.mutex_wait);

        pthread_cond_signal(&shared->clock.cond_wait);

        count++;
    }
}
