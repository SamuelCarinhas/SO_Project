/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "team_manager.h"


/*
* NAME :                            void * car_thread(void * p)
*
* DESCRIPTION :                     Function to handle the Car Thread
*
* PARAMETERS :
*           car_t *       p         pointer to the car
*       
* RETURN :
*          void *                   NULL
*
*/
void * car_thread(void * p) {
    car_t car = *((car_t *) p);
    #ifdef DEBUG
        write_log("DEBUG: Car %d [Team %s] created\n", car.number, car.team_name);
    #endif
    pthread_exit(NULL);
    return NULL;
}

/*
* NAME :                            void team_manager(shared_memory_t * shared_memory, team_t * team)
*
* DESCRIPTION :                     Function to handle Team process (1 per team)
*
* PARAMETERS :
*           shared_memory_t *       shared_memory          pointer to the shared memory
*           team_t *                team                   pointer to the team
*       
* RETURN :
*          void
*
*/
void team_manager(shared_memory_t * shared_memory, team_t * team) {
    #ifdef DEBUG
        write_log("DEBUG: Team %s manager created [%d]\n", team->name, getpid());
    #endif
    while(1) {
        pthread_mutex_lock(&shared_memory->mutex);
        while (team->num_cars == team->res && shared_memory->race_started == 0) {
            pthread_cond_wait(&shared_memory->new_command, &shared_memory->mutex);
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
        #ifdef DEBUG
            write_log("DEBUG: Team %s : Car %d is leaving\n", team->name, team->cars[i].number);
        #endif
        pthread_join(team->cars[i].thread, NULL);
    }
}