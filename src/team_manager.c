/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "team_manager.h"

typedef struct {
    shared_memory_t * shared_memory;
    car_t * car;
    config_t * config;
    int fd;
} arguments_t;

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
    arguments_t arguments = *((arguments_t *) p);
    message_t message;
    car_t * car = arguments.car;
    config_t * config = arguments.config;
    int fd = arguments.fd;

    char buffer[MAX_STRING];

    #ifdef DEBUG
        write_log("DEBUG: Car %d [Team %s] created\n", car->number, car->team->name);
    #endif

    shared_memory_t * shared_memory = arguments.shared_memory;

    pthread_mutex_lock(&shared_memory->mutex);
    while (shared_memory->race_started == 0) {
        pthread_cond_wait(&shared_memory->new_command, &shared_memory->mutex);
    }
    pthread_mutex_unlock(&shared_memory->mutex);

    int total_distance = config->laps * config->lap_distance;
    while(1) {
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!! FAZER ISTO SINCRONIZADO COM AS ESTATISTICAS !!!!!!!!!!!!!!!!!!!!!!!!!!11
        // DE FORMA EFICIENTE =D
        sleep(config->time_units_per_second);
        if(msgrcv(shared_memory->message_queue, &message, sizeof(message_t) - sizeof(long), car->number, IPC_NOWAIT) >= 0) {
            snprintf(buffer, MAX_STRING, "NEW PROBLEN IN CAR %d", car->number);
            write(fd, buffer, MAX_STRING);
            pthread_mutex_lock(&shared_memory->mutex);
            car->status = SAFE_MODE;
            pthread_mutex_unlock(&shared_memory->mutex);
        }

        pthread_mutex_lock(&shared_memory->mutex);
        if(car->status == SAFE_MODE) {
            car->distance += 0.3*car->current_speed;
            car->fuel -= 0.4*car->consuption;
        } else {
            car->fuel -= car->consuption;
            car->distance += car->current_speed;
        }

        if(car->distance >= total_distance) {
            car->status = FINISHED;
            write_log("Car %d finished the race\n", car->number);
            pthread_exit(NULL);
        } else {
            // !!!!!!!!!!! GET THE SPEED IN SAFE MODE !!!!!!!!!!!
            // Fazer tudo...
            int max_laps = (int) (car->fuel / (config->lap_distance/car->speed * car->consuption));

            if(max_laps < 2) {
                car->status = SAFE_MODE;
            }
        }
        pthread_mutex_unlock(&shared_memory->mutex);
    }

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
void team_manager(shared_memory_t * shared_memory, team_t * team, config_t * config, int fd) {
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
        
        arguments_t arguments;
        arguments.shared_memory = shared_memory;
        arguments.config = config;
        arguments.car = get_car(shared_memory, config, team->pos_array, team->res);
        arguments.fd = fd;
        pthread_create(&arguments.car->thread, NULL, car_thread, &arguments);
        team->res++;
    }

    for(int i = 0; i < team->res; i++) {
        pthread_join(get_cars(shared_memory, config)[i].thread, NULL);
        #ifdef DEBUG
            write_log("DEBUG: Team %s : Car %d is leaving\n", team->name, get_car(shared_memory, config, team->pos_array, i)->number);
        #endif
    }
}