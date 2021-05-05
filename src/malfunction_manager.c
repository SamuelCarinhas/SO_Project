/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "malfunction_manager.h"

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
void malfunction_manager(shared_memory_t * shared_memory, config_t * config) {
    #ifdef DEBUG
        write_log("DEBUG: Malfunction manager created [%d]\n", getpid());
    #endif

    int i, j;

    srand(getpid());

    pthread_mutex_lock(&shared_memory->mutex);
    while (shared_memory->race_started == 0) {
        pthread_cond_wait(&shared_memory->new_command, &shared_memory->mutex);
    }

    pthread_mutex_unlock(&shared_memory->mutex);

    message_t message;
    message.malfunction = 1;

    while(1) {
        //printf("Bruh\n");
        //write_log("Gonna sleep for %d seconds...\n", config->time_units_per_second * config->malfunction_time_units);
        sleep(config->time_units_per_second * config->malfunction_time_units);
        for(i = 0; i < shared_memory->num_teams; i++) {
            for(j = 0; j < get_teams(shared_memory)[i].num_cars; j++) {
                car_t * car = get_car(shared_memory, config, i, j);
                message.car_number = car->number;
                int debug = rand() % 100;

                if(debug > car->reliability && car->status == RACE) {
                    msgsnd(shared_memory->message_queue, &message, sizeof(message_t) - sizeof(long), 0);
                    #ifdef DEBUG
                        write_log("DEBUG: Malfunction in car %d\n", message.car_number);
                    #endif
                }
            }
        }
    }
}
