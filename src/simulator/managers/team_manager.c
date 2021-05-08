/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "team_manager.h"

typedef struct argument arguments_t;

struct argument {
    shared_memory_t * shared_memory;
    car_t * car;
    config_t * config;
};

int fd;

void box(int fd, car_t * car, config_t * config) {
    pthread_mutex_lock(&car->team->team_mutex);
    double current_speed = (car->status == SAFE_MODE) ? 0.3*car->speed : car->speed;
    int max_laps = (int) (car->fuel / (config->lap_distance/current_speed * car->consuption));
    if((car->team->box == RESERVED && car->status == SAFE_MODE) || (car->team->box == OPEN && max_laps <= 4)) {
        car->team->box = OCCUPIED;
        car->status = BOX;
        pthread_mutex_unlock(&car->team->team_mutex);
        write_pipe(fd, "CAR %d ENTER THE BOX [TEAM: %s]", car->number, car->team->name);
        usleep((rand() % (config->max_repair_time - config->min_repair_time + 1) + config->min_repair_time ) * 1000000.0/config->time_units_per_second);
        pthread_mutex_lock(&car->team->team_mutex);
        car->fuel = config->fuel_capacity;
        car->status = RACE;
        car->total_boxstops++;
        car->total_refuels++;
        car->team->safe_cars--;
        car->team->box = (car->team->safe_cars > 0) ? RESERVED : OPEN;
        pthread_mutex_unlock(&car->team->team_mutex);
        write_pipe(fd, "CAR %d LEFT THE BOX [TEAM: %s]\n", car->number, car->team->name);
        
    } else {
        pthread_mutex_unlock(&car->team->team_mutex);
    }
}

void car_simulator(void * arg) {
    arguments_t arguments = *((arguments_t *) arg);
    message_t message;
    car_t * car = arguments.car;
    config_t * config = arguments.config;
    shared_memory_t * shared_memory = arguments.shared_memory;
    int total_distance = config->laps * config->lap_distance;

    while(1) {
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!! FAZER ISTO SINCRONIZADO COM AS ESTATISTICAS !!!!!!!!!!!!!!!!!!!!!!!!!!
        // TODO: DE FORMA EFICIENTE =D
        usleep(1000000.0/config->time_units_per_second);
        if(msgrcv(shared_memory->message_queue, &message, sizeof(message_t) - sizeof(long), car->number, IPC_NOWAIT) >= 0) {
            pthread_mutex_lock(&car->team->team_mutex);
            car->status = SAFE_MODE;
            car->total_malfunctions++;
            if(car->team->box == OPEN)
                car->team->box = RESERVED;
            car->team->safe_cars++;
            pthread_mutex_unlock(&car->team->team_mutex);
            write_pipe(fd, "NEW PROBLEM IN CAR %d", car->number);
        }

        if(car->status == SAFE_MODE) {
            car->distance += 0.3*car->speed;
            car->fuel -= 0.4*car->consuption;
        } else {
            car->distance += car->speed;
            car->fuel -= car->consuption;
        }

        if(car->distance >= total_distance) {
            pthread_mutex_lock(&shared_memory->mutex);
            shared_memory->finish_cars++;
            pthread_mutex_unlock(&shared_memory->mutex);
            
            pthread_mutex_lock(&car->team->team_mutex);
            car->status = FINISHED;
            pthread_mutex_unlock(&car->team->team_mutex);

            write_pipe(fd, "CAR %d FINISHED THE RACE", car->number);
            
            pthread_exit(NULL);
        } else {
            int max_laps = (int) (car->fuel / (config->lap_distance/car->speed * car->consuption));

            if(max_laps < 2 && car->status == RACE) {
                pthread_mutex_lock(&car->team->team_mutex);
                car->status = SAFE_MODE;
                if(car->team->box == OPEN)
                    car->team->box = RESERVED;
                car->team->safe_cars++;
                pthread_mutex_unlock(&car->team->team_mutex);
                write_pipe(fd, "CAR %d IS IN SAFE_MODE DUE TO LOW FUEL", car->number);
            }
        }
        //speed and consumption prediction in the next instant
        double current_speed = (car->status == SAFE_MODE) ? 0.3*car->speed : car->speed;
        double current_consumption = (car->status == SAFE_MODE) ? 0.4*car->consuption : car->consuption;

        //check if the car has fuel until the next instant
        if(car->fuel - current_consumption <= 0) {
            sleep(config->time_units_per_second * car->fuel / car->consuption);
            pthread_mutex_lock(&shared_memory->mutex);
            shared_memory->finish_cars++;
            pthread_mutex_unlock(&shared_memory->mutex);
            pthread_mutex_lock(&car->team->team_mutex);
            car->status = GAVE_UP;
            pthread_mutex_unlock(&car->team->team_mutex);
            write_pipe(fd, "CAR %d GAVE UP\n", car->number);
            pthread_exit(NULL);
        }

        //check if the car can reach the box in the next instant
        int laps = (int) (car->distance / config->lap_distance);
        int laps_after = (int)((car->distance + current_speed) / config->lap_distance);
        double distance_until_box = (laps + 1) * config->lap_distance - car->distance;
        double fuel_needed = current_consumption*distance_until_box/current_speed; 
        
        if(laps_after > laps) {
            pthread_mutex_lock(&shared_memory->mutex);

            if(shared_memory->race_started == 0){
                pthread_mutex_unlock(&shared_memory->mutex);
                pthread_mutex_lock(&car->team->team_mutex);
                car->status = FINISHED;
                pthread_mutex_unlock(&car->team->team_mutex);
                return;
            }else
                pthread_mutex_unlock(&shared_memory->mutex);

            if(car->fuel < fuel_needed) {
                pthread_mutex_lock(&shared_memory->mutex);
                shared_memory->finish_cars++;
                pthread_mutex_unlock(&shared_memory->mutex);
                pthread_mutex_lock(&car->team->team_mutex);
                car->status = GAVE_UP;
                pthread_mutex_unlock(&car->team->team_mutex);
                write_pipe(fd, "CAR %d GAVE UP");
                pthread_exit(NULL);
            }
            box(fd, car, config);
            write_debug("CAR %d COMPLETED LAP Nº%d\n", car->number, laps_after);
        }
        
    }
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

    write_debug("CAR %d [TEAM %s] CREATED\n", car->number, car->team->name);

    shared_memory_t * shared_memory = arguments.shared_memory;

    while(1) {
        wait_for_start(shared_memory);
        car_simulator(&arguments);
        // TODO: Sinal para uma COND_VAR para alertar que um um carro deu reset
        // ? NO RESET_TEAM TEMOS QUE CAPUTRAR ESTES SINAIS NA COND_VAR E ESPERAR QUE TODOS OS CARROS DA EQUIPA MANDEM SINAL
    }

    return NULL;
}

void reset_team() {
    
}

void close_team() {
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
    
    signal(SIGINT, close_team);
    signal(SIGUSR1, reset_team);

    write_debug("TEAM MANAGER %s CREATED[%d]\n", team->name, getpid());
    
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
    
    for(int i = 0; i < team->num_cars; i++) {
        pthread_join(get_cars(shared_memory, config)[i].thread, NULL);
        write_debug("TEAM %s : CAR %d IS LEAVING\n", team->name, get_car(shared_memory, config, team->pos_array, i)->number);
    }

    close(fd);
    close_team();
}