/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "team_manager.h"

int fd;
shared_memory_t * shared_memory;
config_t * config;
team_t * team;

void team_function();
void end_team();

int box(car_t * car) {
    pthread_mutex_lock(&car->team->team_mutex);

    double current_speed = (car->status == SAFE_MODE) ? 0.3*car->speed : car->speed;
    int max_laps = (int) (car->fuel / (config->lap_distance/current_speed * car->consumption));

    if((car->team->box == RESERVED && car->status == SAFE_MODE) || (car->team->box == OPEN && max_laps <= 4)) {
        enum car_status join_state = car->status;
        car->team->box = OCCUPIED;
        car->status = BOX;

        pthread_mutex_unlock(&car->team->team_mutex);
        write_pipe(fd, "CAR %d ENTER THE BOX [TEAM: %s]", car->number, car->team->name);
        usleep((rand() % (config->max_repair_time - config->min_repair_time + 1) + config->min_repair_time ) * 1000000.0/config->time_units_per_second);
        pthread_mutex_lock(&car->team->team_mutex);

        car->fuel = config->fuel_capacity;
        car->total_boxstops++;
        car->status = RACE;
        car->total_refuels++;
        if(join_state == SAFE_MODE)
            car->team->safe_cars--;
        car->team->box = (car->team->safe_cars > 0) ? RESERVED : OPEN;

        pthread_mutex_unlock(&car->team->team_mutex);

        write_pipe(fd, "CAR %d LEFT THE BOX [TEAM: %s]\n", car->number, car->team->name);

        return 1;
    } else {
        pthread_mutex_unlock(&car->team->team_mutex);
        
        return 0;
    }
}

void car_simulator(void * arg) {
    car_t * car = (car_t *) arg;
    message_t message;

    int total_distance = config->laps * config->lap_distance;
    double current_speed, current_consumption;

    while(1) {
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

        current_speed = (car->status == SAFE_MODE) ? 0.3*car->speed : car->speed;
        current_consumption = (car->status == SAFE_MODE) ? 0.4*car->consumption : car->consumption;

        //check if the car can reach the box in the next instant
        int laps = (int) (car->distance / config->lap_distance);
        int laps_after = (int)((car->distance + current_speed) / config->lap_distance);
        //double distance_until_box = (laps + 1) * config->lap_distance - car->distance;
        //double fuel_needed = current_consumption*distance_until_box/current_speed;
        
        car->distance += current_speed;
        car->fuel -= current_consumption;

        if(laps_after > laps) {
            pthread_mutex_lock(&shared_memory->mutex);

            if(shared_memory->end_race == 1){
                shared_memory->finish_cars++;
                int rank = ++shared_memory->ranking;
                pthread_mutex_unlock(&shared_memory->mutex);
                pthread_mutex_lock(&car->team->team_mutex);
                car->distance = laps_after * config->lap_distance;
                car->status = FINISHED;
                car->rank = rank;
                pthread_mutex_unlock(&car->team->team_mutex);
                write_pipe(fd, "CAR %d FINISHED THE RACE", car->number);
                pthread_mutex_lock(&shared_memory->mutex);
                if(shared_memory->finish_cars == shared_memory->total_cars) {
                    pthread_mutex_unlock(&shared_memory->mutex);
                    write_pipe(fd, "FINISH");
                }else
                    pthread_mutex_unlock(&shared_memory->mutex);

                pthread_exit(NULL);
            }else
                pthread_mutex_unlock(&shared_memory->mutex);

            write_debug("CAR %d COMPLETED LAP Nº%d\n", car->number, laps_after);
            if(box(car)) {
                pthread_mutex_lock(&shared_memory->mutex);
                if(shared_memory->end_race == 1) {
                    shared_memory->finish_cars++;
                    int rank = ++shared_memory->ranking;
                    int last = shared_memory->finish_cars == shared_memory->total_cars;
                    pthread_mutex_unlock(&shared_memory->mutex);
                    pthread_mutex_lock(&car->team->team_mutex);
                    car->status = FINISHED;
                    car->rank = rank;
                    pthread_mutex_unlock(&car->team->team_mutex);
                    write_pipe(fd, "CAR %d FINISHED THE RACE", car->number);

                    if(last)
                        write_pipe(fd, "FINISH");

                    pthread_exit(NULL);
                }
                
                pthread_mutex_unlock(&shared_memory->mutex);

                continue;
            }
        }

        if(car->fuel <= 0) {
            pthread_mutex_lock(&shared_memory->mutex);
            shared_memory->finish_cars++;
            int last = shared_memory->finish_cars == shared_memory->total_cars;
            pthread_mutex_unlock(&shared_memory->mutex);

            pthread_mutex_lock(&car->team->team_mutex);
            car->fuel = 0;
            car->rank = LAST_RANK;
            car->status = GAVE_UP;
            pthread_mutex_unlock(&car->team->team_mutex);
            write_pipe(fd, "CAR %d GAVE UP\n", car->number);

            if(last) //para sinalizar o ultimo carro a terminar a corrida
                write_pipe(fd, "FINISH");
            pthread_exit(NULL);
        }

        if(car->distance >= total_distance) {
            pthread_mutex_lock(&shared_memory->mutex);
            shared_memory->finish_cars++;
            int rank = ++shared_memory->ranking;
            pthread_mutex_unlock(&shared_memory->mutex);
            
            pthread_mutex_lock(&car->team->team_mutex);
            car->distance = total_distance;
            car->status = FINISHED;
            car->rank = rank;
            pthread_mutex_unlock(&car->team->team_mutex);

            write_pipe(fd, "CAR %d FINISHED THE RACE", car->number);
            
            pthread_exit(NULL);
        } else {
            int max_laps = (int) (car->fuel / (config->lap_distance/car->speed * car->consumption));

            if(max_laps <= 2 && car->status == RACE) {
                pthread_mutex_lock(&car->team->team_mutex);
                car->status = SAFE_MODE;
                if(car->team->box == OPEN)
                    car->team->box = RESERVED;
                car->team->safe_cars++;
                pthread_mutex_unlock(&car->team->team_mutex);
                write_pipe(fd, "CAR %d IS IN SAFE_MODE DUE TO LOW FUEL", car->number);
            }
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
    car_t * car = (car_t *) p;
    write_debug("CAR %d [TEAM %s] CREATED\n", car->number, car->team->name);

    wait_for_start(shared_memory, &team->team_mutex);

    car_simulator(car);

    return NULL;
}

void reset_team() {
    write_debug("TEAM_MANAGER [%s]: RECEIVED SIGUSR1\n", team->name);

    for(int i = 0; i < team->num_cars; i++)
        pthread_join(get_car(shared_memory, config, team->pos_array, i)->thread, NULL);

    for(int i = 0; i < team->num_cars; i++) {
        car_t * car = get_car(shared_memory, config, team->pos_array, i);
        pthread_create(&car->thread, NULL, car_thread, car);
    }
}

void end_team() {
    pthread_mutex_lock(&shared_memory->mutex);
    int restarting = shared_memory->restarting_race;
    pthread_mutex_unlock(&shared_memory->mutex);
    
    if(restarting)
        return;
        
    signal(SIGINT, SIG_IGN);
    write_debug("TEAM_MANAGER [%s]: RECEIVED SIGINT\n", team->name);

    if(shared_memory->race_started == 0)
        exit(0);

    for(int i = 0; i < team->num_cars; i++)
        pthread_join(get_car(shared_memory, config, team->pos_array, i)->thread, NULL);
    write_log("TEAM %s IS LEAVING\n", team->name);
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
void team_manager(shared_memory_t * shared, team_t * t, config_t * conf, int fd_unnamed_pipe) {
    shared_memory = shared;
    config = conf;
    team = t;
    fd = fd_unnamed_pipe;

    write_debug("TEAM MANAGER %s CREATED [%d]\n", team->name, getpid());
    
    team_function();

    close(fd);
}

void team_function() {
    signal(SIGINT, end_team);
    signal(SIGUSR1, reset_team);
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

        car_t * car = get_car(shared_memory, config, team->pos_array, team->res);
        pthread_create(&car->thread, NULL, car_thread, car);
        team->res++;
    }
    
    // Espera por um sinal sobre o fim da corrida
    pthread_mutex_lock(&shared_memory->end_race_mutex);
    pthread_cond_wait(&shared_memory->end_race_cond, &shared_memory->end_race_mutex);
    pthread_mutex_unlock(&shared_memory->end_race_mutex);

    for(int i = 0; i < team->num_cars; i++) {
        pthread_join(get_car(shared_memory, config, team->pos_array, i)->thread, NULL);
        write_debug("TEAM %s CAR %d IS LEAVING\n", team->name, get_car(shared_memory, config, team->pos_array, i)->number);
    }
}