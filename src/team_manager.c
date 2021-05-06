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
} arguments_t;

int fd;

void box(car_t * car, config_t * config) {
    pthread_mutex_lock(&car->team->team_mutex); // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    double current_speed = (car->status == SAFE_MODE) ? 0.3*car->current_speed : car->current_speed;
    int max_laps = (int) (car->fuel / (config->lap_distance/current_speed * car->consuption));
    if((car->team->box == RESERVED && car->status == SAFE_MODE) || (car->team->box == OPEN && max_laps <= 4)) {
        car->team->box = OCCUPIED;
        car->status = BOX;
        pthread_mutex_unlock(&car->team->team_mutex);
        sleep(rand() % (config->max_repair_time - config->min_repair_time + 1) + config->min_repair_time);
        pthread_mutex_lock(&car->team->team_mutex);
        car->fuel = config->fuel_capacity;
        car->status = RACE;
        car->total_boxstops++;
        car->total_refuels++;
        car->team->safe_cars--;
        car->team->box = (car->team->safe_cars > 0) ? RESERVED : OPEN;
        
    } else {
        write_log("CAR %d TRIED TO ENTER THE BOX\n", car->number);
    }
    pthread_mutex_unlock(&car->team->team_mutex);
    
}

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
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!! FAZER ISTO SINCRONIZADO COM AS ESTATISTICAS !!!!!!!!!!!!!!!!!!!!!!!!!!
        // DE FORMA EFICIENTE =D
        sleep(1/config->time_units_per_second);
        if(msgrcv(shared_memory->message_queue, &message, sizeof(message_t) - sizeof(long), car->number, IPC_NOWAIT) >= 0) {
            snprintf(buffer, MAX_STRING, "NEW PROBLEM IN CAR %d", car->number);
            write(fd, buffer, MAX_STRING);
            pthread_mutex_lock(&car->team->team_mutex);//  ! usar um semafro por equipa
            car->status = SAFE_MODE;
            write_log("CAR %d IS IN SAFE MODE DUE TO A MALFUNCTION\n", car->number);
            car->total_malfunctions++;
            if(car->team->box == OPEN) {
                car->team->box = RESERVED;
            }
            car->team->safe_cars++;
            pthread_mutex_unlock(&car->team->team_mutex);
        }

        if(car->status == SAFE_MODE) {
            car->distance += 0.3*car->current_speed;
            car->fuel -= 0.4*car->consuption;
        } else {
            car->distance += car->current_speed;
            car->fuel -= car->consuption;
        }

        if(car->distance >= total_distance) {
            pthread_mutex_lock(&car->team->team_mutex);
            car->status = FINISHED;
            write_log("CAR %d FINISHED THE RACE\n", car->number);
            if(shared_memory->finish_cars == 0) {
                write_log("CAR %d WON THE RACE\n", car->number);
            }
            shared_memory->finish_cars++;
            pthread_mutex_unlock(&car->team->team_mutex);
            pthread_exit(NULL);
        } else {
            int max_laps = (int) (car->fuel / (config->lap_distance/car->speed * car->consuption));

            if(max_laps < 2 && car->status == RACE) {
                pthread_mutex_lock(&car->team->team_mutex);
                write_log("O CARRO %d NÃO TEM GOTA\n", car->number);
                car->status = SAFE_MODE;
                if(car->team->box == OPEN) {
                    car->team->box = RESERVED;
                }
                car->team->safe_cars++;
                pthread_mutex_unlock(&car->team->team_mutex);
            }
        }
        //futuro
        double current_speed = (car->status == SAFE_MODE) ? 0.3*car->current_speed : car->current_speed;
        double current_consumption = (car->status == SAFE_MODE) ? 0.4*car->consuption : car->consuption;
        //pra box
        int laps = (int) (car->distance / config->lap_distance);
        int laps_after = (int)((car->distance + current_speed) / config->lap_distance);
        double distance_until_box = (laps + 1) * config->lap_distance - car->distance;
        double fuel_needed = current_consumption*distance_until_box/current_speed; 

        if(car->fuel - current_consumption <= 0) {
            sleep(config->time_units_per_second * car->fuel / car->consuption);
            pthread_mutex_lock(&car->team->team_mutex);
            car->status = GAVE_UP;
            write_log("CAR %d GAVE UP\n", car->number);
            pthread_mutex_unlock(&car->team->team_mutex);
            pthread_exit(NULL);
        }

        if(laps_after > laps) {
            if(car->fuel < fuel_needed) {
                pthread_mutex_lock(&car->team->team_mutex);
                car->status = GAVE_UP;
                write_log("O CARRO %d MORREU NA PRAIA\n", car->number);
                pthread_mutex_unlock(&car->team->team_mutex);
                pthread_exit(NULL);
            }
            box(car, config);
            #ifdef DEBUG
                write_log("CAR %d COMPLETED LAP Nº%d\n", car->number, laps_after);
            #endif
        }
        
    }

    pthread_exit(NULL);
    return NULL;
}

void clean_team() {
    exit(0);
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
void team_manager(shared_memory_t * shared_memory, team_t * team, config_t * config, int fd_unnamed_pipe) {
    fd = fd_unnamed_pipe;
    
    signal(SIGINT, clean_team);

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
        pthread_create(&arguments.car->thread, NULL, car_thread, &arguments);
        team->res++;
    }

    for(int i = 0; i < team->res; i++) {
        pthread_join(get_cars(shared_memory, config)[i].thread, NULL);
        #ifdef DEBUG
            write_log("DEBUG: Team %s : Car %d is leaving\n", team->name, get_car(shared_memory, config, team->pos_array, i)->number);
        #endif
    }

    while(1) {
        
    }

    close(fd);
}