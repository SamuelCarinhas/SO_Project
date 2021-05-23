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

static void team_function();

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
static void team_function() {
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
            car_args_t * car_args = create_car_args(shared_memory, config, team, car, fd);
            pthread_create(&car->thread, NULL, car_thread, car_args);
            team->res++;
        }
        
        box_args_t * box_args = create_box_args(shared_memory, config, team, fd);

        pthread_create(&box_thread, NULL, box_handler, box_args);

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
                car_args_t * car_args = create_car_args(shared_memory, config, team, car, fd);
                pthread_create(&car->thread, NULL, car_thread, car_args);
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
