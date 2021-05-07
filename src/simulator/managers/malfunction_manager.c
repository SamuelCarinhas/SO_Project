/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "malfunction_manager.h"

shared_memory_t * shared_memory;
config_t * config;

void malfunction_generator();
void malfunction_signal_handler();

/*
* NAME :                            void race_manager(shared_memory_t * shared_memory)
*
* DESCRIPTION :                     Function to handle the Malfunction process
*
* PARAMETERS:
*           shared_memory_t *       shared_memory          pointer to the shared memory
*       
* RETURN :
*          void
*
* TODO :                            EVERYTHING
*
*/
void malfunction_manager(shared_memory_t * shared, config_t * conf) {
    write_debug("MALFUNCTION MANAGER CREATED [%d]\n", getpid());
    signal(SIGINT, malfunction_signal_handler);
    //signal(SIGUSR1, malfunction_signal_handler);
    shared_memory = shared;
    config = conf;
    malfunction_generator();
}

void malfunction_generator() {
    int i, j, rand_num;
    
    srand(getpid());

    pthread_mutex_lock(&shared_memory->mutex);
    while (shared_memory->race_started == 0)
        pthread_cond_wait(&shared_memory->new_command, &shared_memory->mutex);
    pthread_mutex_unlock(&shared_memory->mutex);

    message_t message;
    message.malfunction = 1;

    while(1) {
        usleep(1.0/config->time_units_per_second * config->malfunction_time_units * 1000000);
        for(i = 0; i < shared_memory->num_teams; i++) {
            for(j = 0; j < get_teams(shared_memory)[i].num_cars; j++) {
                car_t * car = get_car(shared_memory, config, i, j);
                message.car_number = car->number;
                // GENERATE A RANDOM PROBABILITY
                rand_num = rand() % 100;
                pthread_mutex_lock(&car->team->team_mutex);
                // IF THE RANDOM NUMBER IS GREATER THEN THE CAR RELIABILITY AND THE CAR IS IN RACE MODE, WE WILL GENERATE A MALFUNCTION 
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

void malfunction_signal_handler() {
    exit(0);
    /*
    pthread_mutex_lock(&shared_memory->mutex);
    shared_memory->race_started = 0;
    pthread_mutex_unlock(&shared_memory->mutex);
    malfunction_generator();
    */
}
