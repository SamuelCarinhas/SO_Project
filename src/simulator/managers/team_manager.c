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
pthread_t box_thread;

void team_function();
void end_team();
void update_box();
int join_box(car_t * car);

void car_simulator(void * arg) {
    car_t * car = (car_t *) arg;
    message_t message;

    int total_distance = config->laps * config->lap_distance;
    double current_speed, current_consumption;

    while(1) {
        sync_sleep(shared_memory, 1);
        if(msgrcv(shared_memory->message_queue, &message, sizeof(message_t) - sizeof(long), car->number, IPC_NOWAIT) >= 0) {
            pthread_mutex_lock(&team->team_mutex);
            car->status = SAFE_MODE;
            car->total_malfunctions++;
            team->safe_cars++;
            pthread_mutex_unlock(&team->team_mutex);
            update_box();
            write_pipe(fd, "NEW PROBLEM IN CAR %d", car->number);
        }

        current_speed = (car->status == SAFE_MODE) ? 0.3*car->speed : car->speed;
        current_consumption = (car->status == SAFE_MODE) ? 0.4*car->consumption : car->consumption;

        int laps = (int) (car->distance / config->lap_distance);
        int laps_after = (int)((car->distance + current_speed) / config->lap_distance);
        
        car->distance += current_speed;
        car->fuel -= current_consumption;

        if(car->fuel <= 0) {
            pthread_mutex_lock(&shared_memory->mutex);
            shared_memory->finish_cars++;
            pthread_mutex_unlock(&shared_memory->mutex);

            pthread_mutex_lock(&team->team_mutex);
            car->fuel = 0;
            car->rank = LAST_RANK;
            car->status = GAVE_UP;
            pthread_mutex_unlock(&team->team_mutex);

            write_pipe(fd, "CAR %d GAVE UP\n", car->number);
            
            update_box();

            pthread_exit(NULL);
        }

        if(car->distance >= total_distance) {
            pthread_mutex_lock(&shared_memory->mutex);
            shared_memory->finish_cars++;
            int rank = ++shared_memory->ranking;
            pthread_mutex_unlock(&shared_memory->mutex);
            
            pthread_mutex_lock(&team->team_mutex);
            car->distance = total_distance;
            car->status = FINISHED;
            car->rank = rank;
            pthread_mutex_unlock(&team->team_mutex);

            write_pipe(fd, "CAR %d FINISHED THE RACE", car->number);
            
            update_box();
            
            pthread_exit(NULL);
        } else {
            int max_laps = (int) (car->fuel / (config->lap_distance/car->speed * car->consumption));

            if(max_laps <= 2 && car->status == RACE) {
                pthread_mutex_lock(&team->team_mutex);
                car->status = SAFE_MODE;
                team->safe_cars++;
                pthread_mutex_unlock(&team->team_mutex);
                update_box();
                write_pipe(fd, "CAR %d IS IN SAFE_MODE DUE TO LOW FUEL", car->number);
            }
        }

        if(laps_after > laps) {
            pthread_mutex_lock(&shared_memory->mutex);

            if(shared_memory->end_race == 1){
                shared_memory->finish_cars++;
                int rank = ++shared_memory->ranking;
                int last = shared_memory->finish_cars == shared_memory->total_cars;
                pthread_mutex_unlock(&shared_memory->mutex);
                pthread_mutex_lock(&team->team_mutex);
                car->distance = laps_after * config->lap_distance;
                car->status = FINISHED;
                car->rank = rank;
                pthread_mutex_unlock(&team->team_mutex);

                write_pipe(fd, "CAR %d FINISHED THE RACE", car->number);

                if(last) {
                    write_pipe(fd, "FINISH");
                }
                
                update_box();

                pthread_exit(NULL);
            }else
                pthread_mutex_unlock(&shared_memory->mutex);

            write_debug("CAR %d COMPLETED LAP Nº%d\n", car->number, laps_after);
            if(join_box(car)) {
                pthread_mutex_lock(&shared_memory->mutex);
                int end_race = shared_memory->end_race;
                pthread_mutex_unlock(&shared_memory->mutex);
                if(end_race) {
                    pthread_mutex_lock(&shared_memory->mutex);
                    shared_memory->finish_cars++;
                    int rank = ++shared_memory->ranking;
                    int last = shared_memory->finish_cars == shared_memory->total_cars;
                    pthread_mutex_unlock(&shared_memory->mutex);

                    pthread_mutex_lock(&team->team_mutex);
                    car->status = FINISHED;
                    car->rank = rank;
                    pthread_mutex_unlock(&team->team_mutex);

                    write_pipe(fd, "CAR %d FINISHED THE RACE", car->number);

                    if(last) {
                        write_pipe(fd, "FINISH");
                    }
                    
                    update_box();

                    pthread_exit(NULL);
                }

                continue;
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
    write_debug("CAR %d [TEAM %s] CREATED\n", car->number, team->name);

    wait_for_start(shared_memory, &shared_memory->mutex);

    car_simulator(car);

    return NULL;
}

void update_box() {
    pthread_mutex_lock(&team->team_mutex);
    team->box.request_reservation = 1;
    pthread_mutex_unlock(&team->team_mutex);
    pthread_cond_signal(&team->box.request);
}

int join_box(car_t * car) {
    double current_speed = (car->status == SAFE_MODE) ? 0.3*car->speed : car->speed;
    int max_laps = (int) (car->fuel / (config->lap_distance/current_speed * car->consumption));
    if((team->box.status == RESERVED && car->status == SAFE_MODE) || (team->box.status == OPEN && max_laps <= 4)) {
        if(pthread_mutex_trylock(&team->box.mutex) == 0) {
            team->box.car = car;
            team->box.has_car = 1;
            pthread_cond_signal(&team->box.request);

            pthread_mutex_lock(&team->box.leave_mutex);
            pthread_cond_wait(&team->box.car_leave, &team->box.leave_mutex);
            pthread_mutex_unlock(&team->box.leave_mutex);
            return 1;
        }       
    }
    return 0;
}

void * box_handler() {
    while(1) {
        pthread_mutex_lock(&shared_memory->mutex);
        int finish = !shared_memory->race_started;
        pthread_mutex_unlock(&shared_memory->mutex);

        if(finish)
            break;

        pthread_mutex_lock(&team->team_mutex);
        if(team->box.request_reservation){
            team->box.request_reservation = 0;
            team->box.status = (team->safe_cars > 0) ? RESERVED : OPEN;
        }
        pthread_mutex_unlock(&team->team_mutex);

        if(!team->box.has_car) {
            pthread_mutex_lock(&team->box.join_mutex);
            pthread_cond_wait(&team->box.request, &team->box.join_mutex);
            pthread_mutex_unlock(&team->box.join_mutex);
        }

        pthread_mutex_lock(&team->team_mutex);
        int res = team->box.request_reservation;
        if(res){
            team->box.request_reservation = 0;
            team->box.status = (team->safe_cars > 0) ? RESERVED : OPEN;
        }
        pthread_mutex_unlock(&team->team_mutex);

        if(res)
            continue;

        pthread_mutex_lock(&team->team_mutex);
        team->box.status = OCCUPIED;
        enum car_status join_state = team->box.car->status;
        team->box.car->status = BOX;
        pthread_mutex_unlock(&team->team_mutex);

        write_pipe(fd, "CAR %d ENTER THE BOX [TEAM: %s]", team->box.car->number, team->name);
        
        sync_sleep(shared_memory, (rand() % (config->max_repair_time - config->min_repair_time + 1) + config->min_repair_time));
        pthread_mutex_lock(&team->team_mutex);
        team->box.car->fuel = config->fuel_capacity;
        team->box.car->total_boxstops++;
        team->box.car->status = RACE;
        team->box.car->total_refuels++;
        if(join_state == SAFE_MODE)
            team->safe_cars--;
        team->box.status = (team->safe_cars > 0) ? RESERVED : OPEN;
        team->box.has_car = 0;
        pthread_mutex_unlock(&team->team_mutex);

        write_pipe(fd, "CAR %d LEFT THE BOX [TEAM: %s]", team->box.car->number, team->name);

        pthread_cond_signal(&team->box.car_leave);

        pthread_mutex_unlock(&team->box.mutex);
    }

    pthread_exit(NULL);
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

    init_mutex_proc(&team->box.mutex);
    init_mutex_proc(&team->box.join_mutex);
    init_mutex_proc(&team->box.leave_mutex);
    init_cond_proc(&team->box.request);
    init_cond_proc(&team->box.car_leave);
    team->box.request_reservation = 0;
    team->box.car = NULL;
    team->box.has_car = 0;

    write_debug("TEAM MANAGER %s CREATED [%d]\n", team->name, getpid());
    
    team_function();

    close(fd);
}

void team_function() {
    signal(SIGINT, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    while(1) {
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
        
        pthread_create(&box_thread, NULL, box_handler, NULL);

        pthread_join(box_thread, NULL);
        for(int i = 0; i < team->num_cars; i++) {
            pthread_join(get_car(shared_memory, config, team->pos_array, i)->thread, NULL);
            write_debug("TEAM %s CAR %d IS LEAVING\n", team->name, get_car(shared_memory, config, team->pos_array, i)->number);
        }

        break;
    }

    destroy_mutex_proc(&team->box.mutex);
    destroy_mutex_proc(&team->box.join_mutex);
    destroy_mutex_proc(&team->box.leave_mutex);
    destroy_cond_proc(&team->box.request);
    destroy_cond_proc(&team->box.car_leave);
}
