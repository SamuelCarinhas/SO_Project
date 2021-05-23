#include "box_thread.h"

shared_memory_t * shared_memory;
team_t * team;
config_t * config;
int fd;

/**
 * @brief Wakes up the box to check for new changes
 * 
 */
void update_box() {
    pthread_mutex_lock(&team->team_mutex);
    team->box.request_reservation = 1;
    pthread_mutex_unlock(&team->team_mutex);
    pthread_cond_signal(&team->box.request);
}

/**
 * @brief Tries to take the car to the box
 * 
 * @param car Car pointer
 * @return int Logical value
 * 1 If the car got the box reservation
 * 0 Otherwise
 */
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

/**
 * @brief Create a box args structure
 * 
 * @param shared_memory Pointer to the shared memory structure
 * @param config Pointer to the config structure
 * @param team Pointer to the team strucutre 
 * @return box_args_t* Box arguments structure
 */
box_args_t * create_box_args(shared_memory_t * shared_memory, config_t * config, team_t * team, int fd) {
    box_args_t * args = (box_args_t *) malloc(sizeof(box_args_t));
    args->shared_memory = shared_memory;
    args->config = config;
    args->team = team;
    args->fd = fd;
    return args;
}

/**
 * @brief Handle box requests. Requests can be to update the box status
 * if something has changed, for example: restarting or ending race. And
 * the requests can also be to fix cars and then it will simulate the time
 * needed to repair or refuel the car.
 * 
 * @param args Shared memory pointer, config pointer and team pointer
 * @return void* 
 */
void * box_handler(void * args) {
    box_args_t * box_args = (box_args_t *) args;
    shared_memory = box_args->shared_memory;
    config = box_args->config;
    team = box_args->team;
    fd = box_args->fd;
    free(box_args);
    while(1) {
        pthread_mutex_lock(&shared_memory->mutex);
        int finish = shared_memory->total_cars == shared_memory->finish_cars;
        int exit = shared_memory->race_started == 0 && shared_memory->end_race == 1 && shared_memory->restarting_race == 0;
        pthread_mutex_unlock(&shared_memory->mutex);

        if(finish || exit)
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
        pthread_mutex_lock(&team->box.car->car_mutex);
        enum car_status join_state = team->box.car->status;
        team->box.car->status = BOX;
        pthread_mutex_unlock(&team->box.car->car_mutex);
        pthread_mutex_unlock(&team->team_mutex);

        write_pipe(fd, "CAR %d ENTER THE BOX [TEAM: %s]", team->box.car->number, team->name);
        
        if(team->box.car->problem) {
            sync_sleep(shared_memory, rand() % (config->max_repair_time - config->min_repair_time + 1) + config->min_repair_time);
            write_pipe(fd, "[TEAM %s] BOX REPAIRED THE CAR %d\n",team->name,  team->box.car->number);
        } else {
            sync_sleep(shared_memory, 2);
            write_pipe(fd, "[TEAM %s] BOX REFUELED THE CAR %d\n",team->name, team->box.car->number);
        }

        pthread_mutex_lock(&team->box.car->car_mutex);
        team->box.car->fuel = config->fuel_capacity;
        team->box.car->total_boxstops++;
        team->box.car->status = RACE;
        team->box.car->total_refuels++;
        team->box.car->problem = 0;
        pthread_mutex_unlock(&team->box.car->car_mutex);
        pthread_mutex_lock(&team->team_mutex);
        team->box.has_car = 0;
        team->box.status = (team->safe_cars > 0) ? RESERVED : OPEN;
        if(join_state == SAFE_MODE)
            team->safe_cars--;
        pthread_mutex_unlock(&team->team_mutex);
        write_pipe(fd, "CAR %d LEFT THE BOX [TEAM: %s]", team->box.car->number, team->name);

        pthread_cond_signal(&team->box.car_leave);

        pthread_mutex_unlock(&team->box.mutex);
    }

    pthread_exit(NULL);
}
