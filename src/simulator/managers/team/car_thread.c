#include "car_thread.h"

team_t * team;
config_t * config;
shared_memory_t * shared_memory;
int fd;
static void car_simulator(void * arg);

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
 * @param args Car pointer, shared memory pointer, config pointer and team pointer
 * @return void* 
 */
void * car_thread(void * args) {
    car_args_t * car_args = (car_args_t *) args;
    shared_memory = car_args->shared_memory;
    config = car_args->config;
    team = car_args->team;
    fd = car_args->fd;
    car_t * car = car_args->car;
    free(car_args);
    write_debug("CAR %d [TEAM %s] CREATED\n", car->number, team->name);

    if (wait_for_start(shared_memory))
        return NULL;

    car_simulator(car);

    return NULL;
}



/**
 * @brief Create a car args structure
 * 
 * @param shared_memory Pointer to the shared memory structure
 * @param config Pointer to the config structure
 * @param team Pointer to the team strucutre 
 * @param car Pointer to the car structure
 * @return car_args_t* Car arguments structure
 */
car_args_t * create_car_args(shared_memory_t * shared_memory, config_t * config, team_t * team, car_t * car, int fd) {
    car_args_t * args = (car_args_t *) malloc(sizeof(car_args_t));
    args->shared_memory = shared_memory;
    args->config = config;
    args->team = team;
    args->car = car;
    args->fd = fd;
    return args;
}