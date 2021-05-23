/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "team_manager.h"

int fd;

team_t * team;
config_t * config;
shared_memory_t * shared_memory;

pthread_t box_thread;

static int join_box(car_t * car);

static void update_box();
static void team_function();
static void car_simulator(void * arg);

static void * box_handler();
static void * car_thread(void * arg);

/**
 * @brief Simulates the car behavior during the race. Receives malfunctions
 * from the malfunction manager, changes the car status when receiving  a
 * malfunction or having low fuel and requesting access to the box.
 * 
 * @param arg Pointer to car
 */
static void car_simulator(void * arg) {
    car_t * car = (car_t *) arg;
    message_t message;

    int total_distance = config->laps * config->lap_distance;
    double current_speed, current_consumption;

    while(1) {
        sync_sleep(shared_memory, 1);
        
        if(receive_message(shared_memory->message_queue, &message, car->number) >= 0) {
            pthread_mutex_lock(&car->car_mutex);
            car->status = SAFE_MODE;
            car->problem = 1;
            car->total_malfunctions++;
            pthread_mutex_unlock(&car->car_mutex);

            pthread_mutex_lock(&team->team_mutex);
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
            pthread_mutex_lock(&car->car_mutex);
            car->fuel = 0;
            car->rank = LAST_RANK;
            car->status = GAVE_UP;
            pthread_mutex_unlock(&car->car_mutex);

            write_pipe(fd, "CAR %d GAVE UP\n", car->number);
            
            update_box();

            pthread_exit(NULL);
        }

        if(car->distance >= total_distance) {
            pthread_mutex_lock(&shared_memory->mutex);
            int rank = ++shared_memory->ranking;
            pthread_mutex_unlock(&shared_memory->mutex);
            
            pthread_mutex_lock(&car->car_mutex);
            car->distance = total_distance;
            car->status = FINISHED;
            car->rank = rank;
            pthread_mutex_unlock(&car->car_mutex);

            write_pipe(fd, "CAR %d FINISHED THE RACE", car->number);
            
            update_box();
            
            pthread_exit(NULL);
        } else {
            int max_laps = (int) (car->fuel / (config->lap_distance/car->speed * car->consumption));

            if(max_laps <= 2 && car->status == RACE) {
                pthread_mutex_lock(&car->car_mutex);
                car->status = SAFE_MODE;
                pthread_mutex_unlock(&car->car_mutex);
                pthread_mutex_lock(&team->team_mutex);
                team->safe_cars++;
                pthread_mutex_unlock(&team->team_mutex);
                update_box();
                write_pipe(fd, "CAR %d IS IN SAFE_MODE DUE TO LOW FUEL", car->number);
            }
        }

        if(laps_after > laps) {
            pthread_mutex_lock(&shared_memory->mutex);

            if(shared_memory->end_race == 1){
                int rank = ++shared_memory->ranking;
                pthread_mutex_unlock(&shared_memory->mutex);

                pthread_mutex_lock(&car->car_mutex);
                car->distance = laps_after * config->lap_distance;
                car->status = FINISHED;
                car->rank = rank;
                pthread_mutex_unlock(&car->car_mutex);

                write_pipe(fd, "CAR %d FINISHED THE RACE", car->number);
                
                update_box();

                pthread_exit(NULL);
            }else
                pthread_mutex_unlock(&shared_memory->mutex);

            write_pipe(fd, "CAR %d COMPLETED LAP Nº%d\n", car->number, laps_after);
            if(join_box(car)) {
                pthread_mutex_lock(&shared_memory->mutex);
                int end_race = shared_memory->end_race;
                pthread_mutex_unlock(&shared_memory->mutex);
                if(end_race) {
                    pthread_mutex_lock(&shared_memory->mutex);
                    int rank = ++shared_memory->ranking;
                    pthread_mutex_unlock(&shared_memory->mutex);

                    pthread_mutex_lock(&car->car_mutex);
                    car->status = FINISHED;
                    car->rank = rank;
                    pthread_mutex_unlock(&car->car_mutex);

                    write_pipe(fd, "CAR %d FINISHED THE RACE", car->number);
                    
                    update_box();

                    pthread_exit(NULL);
                }

                continue;
            }
        }
    }
}

/**
 * @brief Waits for the race to start and calls the car_simulator
 * function
 * 
 * @param arg Car pointer
 * @return void* 
 */
static void * car_thread(void * arg) {
    car_t * car = (car_t *) arg;
    write_debug("CAR %d [TEAM %s] CREATED\n", car->number, team->name);

    if (wait_for_start(shared_memory))
        return NULL;

    car_simulator(car);

    return NULL;
}

/**
 * @brief Wakes up the box to check for new changes
 * 
 */
static void update_box() {
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
static int join_box(car_t * car) {
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
 * @brief Handle box requests. Requests can be to update the box status
 * if something has changed, for example: restarting or ending race. And
 * the requests can also be to fix cars and then it will simulate the time
 * needed to repair or refuel the car.
 * 
 * @return void* 
 */
static void * box_handler() {
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

/**
 * @brief Initializes the team box mutexes and condition variables and calls
 * the team_function function.
 * 
 * @param shared Pointer to the shared memory structure
 * @param t Pointer to the team structure
 * @param conf Pointer to the config structure
 * @param fd_unnamed_pipe File descriptor of the team unnamed pipe
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

/**
 * @brief Handles the new cars requests and create the box and car threads.
 * Check if when the race end it needs to be restarted and cleans the resources.
 * 
 */
void team_function() {
    signal(SIGINT, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    while(1) {
        while(1) {
            pthread_mutex_lock(&shared_memory->mutex);
            int exit = 0;
            while ((team->num_cars == team->res && shared_memory->race_started == 0) || shared_memory->restarting_race) {
                pthread_cond_wait(&shared_memory->new_command, &shared_memory->mutex);

                if(shared_memory->race_started == 0 && shared_memory->end_race == 1 && shared_memory->restarting_race == 0) {
                    exit = 1;
                    break;
                }
            }
            int can_start = shared_memory->race_started;
            pthread_mutex_unlock(&shared_memory->mutex);

            if(can_start || exit) {
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

        pthread_mutex_lock(&shared_memory->mutex);
        int restart = shared_memory->restarting_race;
        pthread_mutex_unlock(&shared_memory->mutex);
        
        if(restart){
            pthread_mutex_lock(&shared_memory->mutex_reset);
            shared_memory->waiting_for_reset++;
            pthread_mutex_unlock(&shared_memory->mutex_reset);
            pthread_cond_signal(&shared_memory->reset_race);

            for(int i = 0; i < team->num_cars; i++){
                car_t * car = get_car(shared_memory, config, team->pos_array, i);
                pthread_create(&car->thread, NULL, car_thread, car);
            }
        } else {
            break;
        }
    }
    
    destroy_mutex_proc(&team->box.mutex);
    destroy_mutex_proc(&team->box.join_mutex);
    destroy_mutex_proc(&team->box.leave_mutex);
    destroy_cond_proc(&team->box.request);
    destroy_cond_proc(&team->box.car_leave);

    write_debug("TEAM MANAGER %s LEFT\n", team->name);
}
