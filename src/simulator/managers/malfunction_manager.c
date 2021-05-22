/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "malfunction_manager.h"

shared_memory_t * shared_memory;
config_t * config;
static int malfunction_generator();

/**
 * @brief Handle the malfunction manager
 * 
 * @param shared Pointer to the shared memory structure
 * @param conf Pointer to the config structure
 */
void malfunction_manager(shared_memory_t * shared, config_t * conf) {
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);
    write_debug("MALFUNCTION MANAGER CREATED [%d]\n", getpid());
    shared_memory = shared;
    config = conf;

    while(1) {
        if(malfunction_generator())
            break;
        pthread_mutex_lock(&shared->mutex_reset);
        shared->waiting_for_reset++;
        pthread_mutex_unlock(&shared->mutex_reset);
        pthread_cond_signal(&shared->reset_race);
        printf("MALFUNCTION RESTART\n");
    }

    write_debug("MALFUNCTION MANAGER LEFT\n");
}

/**
 * @brief Generate multiple malfunctions to the race cars per time units.
 * 
 * @return int Logic value if the malfunction need to restart or can be closed
 * 1 -> End malfunction
 * 0 -> Restart malfunction
 */
static int malfunction_generator() {
    int i, j, rand_num;
    
    srand(getpid());

    if(wait_for_start(shared_memory)) {
        return 1;
    }

    message_t message;
    message.malfunction = 1;

    while(1) {
        sync_sleep(shared_memory, config->malfunction_time_units);

        pthread_mutex_lock(&shared_memory->mutex);
        int finish = shared_memory->total_cars == shared_memory->finish_cars && shared_memory->restarting_race == 0;
        int restart = shared_memory->total_cars == shared_memory->finish_cars && shared_memory->restarting_race == 1;
        pthread_mutex_unlock(&shared_memory->mutex);
        
        if(finish || restart)
            return finish;
        
        for(i = 0; i < shared_memory->num_teams; i++) {
            for(j = 0; j < get_teams(shared_memory)[i].num_cars; j++) {
                car_t * car = get_car(shared_memory, config, i, j);
                message.car_number = car->number;
                rand_num = rand() % 100;
                pthread_mutex_lock(&car->team->team_mutex);
                if(rand_num > car->reliability && car->status == RACE) {
                    pthread_mutex_unlock(&car->team->team_mutex);
                    send_message(shared_memory->message_queue, &message);
                    write_debug("MALFUNCTION IN CAR %d\n", message.car_number);
                } else {
                    pthread_mutex_unlock(&car->team->team_mutex);
                }
            }
        }
    }
}
