/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "clock.h"

/**
 * @brief Simulate a global clock that notify every process every
 * T time units
 * 
 * @param shared Pointer to the shared memory structure
 * @param config Pointer to the config structure
 */
void sync_clock(shared_memory_t * shared, config_t * config) {
    if(wait_for_start(shared))
        return;
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);
    while(1) {
        usleep(1.0/config->time_units_per_second*1000000);

        pthread_mutex_lock(&shared->mutex);
        int finish = shared->end_race == 1 && shared->race_started == 0;
        int res = shared->total_cars == shared->finish_cars && shared->restarting_race == 0;
        pthread_mutex_unlock(&shared->mutex);

        pthread_mutex_lock(&shared->clock.mutex);
        pthread_cond_broadcast(&shared->clock.time_cond);
        pthread_mutex_unlock(&shared->clock.mutex);
        
        if(finish || res) {
            break;
        }

    }
    write_debug("CLOCK LEFT\n");
}

/**
 * @brief Wait for N clock signals to simulate the default
 * sleep function.
 * 
 * @param shared Pointer to the shared memory structure
 * @param time Time units to sleep
 */
void sync_sleep(shared_memory_t * shared, int time) {
    int count = 0;

    pthread_mutex_lock(&shared->clock.mutex);
    while(count < time) {
        pthread_cond_wait(&shared->clock.time_cond, &shared->clock.mutex);
        count++;

        pthread_mutex_lock(&shared->mutex);
        int finish = shared->end_race == 1 && shared->race_started == 0;
        int res = shared->total_cars == shared->finish_cars && shared->restarting_race == 0;
        pthread_mutex_unlock(&shared->mutex);

        if(finish || res)
            break;
    }
    pthread_mutex_unlock(&shared->clock.mutex);
}