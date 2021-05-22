/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "clock.h"


void sync_clock(shared_memory_t * shared, config_t * config) {
    if(wait_for_start(shared, &shared->mutex))
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

void sync_sleep(shared_memory_t * shared, int time) {
    int count = 0;
    while(count < time) {
        count++; 

        pthread_mutex_lock(&shared->mutex);
        int finish = shared->end_race == 1 && shared->race_started == 0;
        int res = shared->total_cars == shared->finish_cars && shared->restarting_race == 0;
        pthread_mutex_unlock(&shared->mutex);

        if(finish || res)
            break;

        pthread_mutex_lock(&shared->clock.mutex);
        pthread_cond_wait(&shared->clock.time_cond, &shared->clock.mutex);
        pthread_mutex_unlock(&shared->clock.mutex);
    }
}


/*
void sync_clock(shared_memory_t * shared, config_t * config) {
    wait_for_start(shared, &shared->mutex);
    signal(SIGINT, SIG_IGN);
    while(1) { 
        pthread_mutex_lock(&shared->mutex);
        int res = shared->end_race == 1 && shared->race_started == 0;
        pthread_mutex_lock(&shared->mutex);

        if(res)
            break;

        pthread_mutex_lock(&shared->clock.wait_mutex);
        while(shared->clock.waiting < shared->total_cars - shared->finish_cars + shared->race_started) {
            pthread_cond_wait(&shared->clock.wait_cond, &shared->clock.wait_mutex);
        }
        pthread_mutex_unlock(&shared->clock.wait_mutex);

        usleep(1.0/config->time_units_per_second*1000000);

        pthread_mutex_lock(&shared->clock.mutex);
        pthread_cond_broadcast(&shared->clock.time_cond);
        pthread_mutex_unlock(&shared->clock.mutex);

    }
}

void sync_sleep(shared_memory_t * shared, int time) {
    int count = 0;
    while(count < time) {
        count++;

        pthread_mutex_lock(&shared->clock.wait_mutex);
        shared->clock.waiting++;
        pthread_cond_signal(&shared->clock.wait_cond);
        pthread_mutex_unlock(&shared->clock.wait_mutex);


        pthread_mutex_lock(&shared->clock.mutex);
        pthread_cond_wait(&shared->clock.time_cond, &shared->clock.mutex);
        pthread_mutex_unlock(&shared->clock.mutex);

        pthread_mutex_lock(&shared->clock.wait_mutex);
        shared->clock.waiting--;
        pthread_mutex_unlock(&shared->clock.wait_mutex);
    }
}
*/