/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "memory.h"

char * car_string[] = {
    "RACE",
    "SAFE_MODE",
    "BOX",
    "GAVE_UP",
    "FINISHED"
};

/**
 * @brief Displays the statistics of the race
 * 
 * @param shared_memory Pointer to the shared memory structure
 * @param config Pointer to the config structure
 * @param finish Logical value if the function was called by a interruption or signal TSTP
 */
void show_statistics(shared_memory_t * shared_memory, config_t * config, int finish) {
    pthread_mutex_lock(&shared_memory->mutex);
    int num_teams = shared_memory->num_teams;
    int total_cars = shared_memory->total_cars;
    int finish_cars = shared_memory->finish_cars;
    pthread_mutex_unlock(&shared_memory->mutex);

    team_t * teams = get_teams(shared_memory);
    for(int i = 0; i < num_teams; i++) {
        team_t * team = &teams[i];
        for(int j = 0; j < team->num_cars; j++)
            pthread_mutex_lock(&get_car(shared_memory, config, team->pos_array, j)->car_mutex);
    }
    
    int max_statistics = (total_cars >= TOP_STATISTICS) ? TOP_STATISTICS : total_cars;
    car_t car_ranking[total_cars];
    for(int i = 0; i < total_cars; i++) {
        car_ranking[i].distance = -1;
        car_ranking[i].rank = LAST_RANK;
    }

    car_t * car;
    int total_malfunctions = 0, total_refuels = 0;
    for(int i = 0; i < num_teams; i++) {
        for(int j = 0; j < teams[i].num_cars; j++) {
            car = get_car(shared_memory, config, i, j);
            int l;
            total_malfunctions += car->total_malfunctions;
            total_refuels +=car->total_refuels;
            for(l = 0;
                (l < total_cars) &&((car->distance < car_ranking[l].distance) || (car->distance == car_ranking[l].distance && car->rank > car_ranking[l].rank)); l++);
            if(l == total_cars) continue;
            for(int k = total_cars - 1; k > l; k--)
                car_ranking[k] = car_ranking[k-1];
            car_ranking[l] = *car;
        }
    }
    
    for(int i = 0; i < num_teams; i++) {
        team_t * team = &teams[i];
        for(int j = 0; j < team->num_cars; j++)
            pthread_mutex_unlock(&get_car(shared_memory, config, team->pos_array, j)->car_mutex);
    }
 
    char buffer[MAX_STRING*6] = "\n-------------------------------------------------------------------------------\n                                  STATISTICS\n";
    char buffer2[MAX_STRING];
    snprintf(buffer2, MAX_STRING*2, "-------------------------------------------------------------------------------\n");
    strcat(buffer, buffer2);
    snprintf(buffer2, MAX_STRING*2, "|   RANK   |  CAR  |        TEAM        | LAPS | STOPS |   FUEL  |   STATUS   |\n");
    strcat(buffer, buffer2);
    snprintf(buffer2, MAX_STRING*2, "-------------------------------------------------------------------------------\n");
    strcat(buffer, buffer2);
    for(int i = 0; i < max_statistics; i++) {
        snprintf(buffer2, MAX_STRING*2, "| %8d | %5d | %*s | %4d | %5d | %7.1f | %10s |\n", (car_ranking[i].status == GAVE_UP) ? total_cars : i+1, car_ranking[i].number, MAX_TEAM_NAME + 2, car_ranking[i].team->name, (int)(car_ranking[i].distance/config->lap_distance), car_ranking[i].total_boxstops, car_ranking[i].fuel, car_string[car_ranking[i].status]);
        strcat(buffer, buffer2);
    }
    

    if(total_cars > TOP_STATISTICS) {
        snprintf(buffer2, MAX_STRING*2, "-------------------------------------------------------------------------------\n");
        strcat(buffer, buffer2);
        snprintf(buffer2, MAX_STRING*2, "| %8d | %5d | %*s | %4d | %5d | %7.1f | %10s | \n", total_cars, car_ranking[total_cars-1].number, MAX_TEAM_NAME + 2, car_ranking[total_cars -1].team->name, (int)(car_ranking[total_cars -1].distance/config->lap_distance), car_ranking[total_cars -1].total_boxstops, car_ranking[total_cars -1].fuel, car_string[car_ranking[total_cars -1].status]);
        strcat(buffer, buffer2);
    }

    snprintf(buffer2, MAX_STRING*2, "-------------------------------------------------------------------------------\n| TOTAL MALFUNCTIONS: %55d |\n| TOTAL REFUELS: %60d |\n| TOTAL RACING CARS: %56d |\n-------------------------------------------------------------------------------\n", total_malfunctions, total_refuels, total_cars - finish_cars);
    strcat(buffer, buffer2);

    write_log(buffer);

    if(finish)
        write_log("CAR %d (TEAM %s) WINS THE RACE.\n", car_ranking[0].number, car_ranking[0].team->name);
}

/**
 * @brief Initializes the car variables
 * 
 * @param car Pointer to the car structure
 * @param config Pointer to the config structure
 */
void init_car(car_t * car, config_t * config) {
    car->status = RACE;
    car->fuel = config->fuel_capacity;
    car->distance = 0;
    car->rank = LAST_RANK;
    car->total_malfunctions = 0;
    car->total_refuels = 0;
    car->total_boxstops = 0;
    car->problem = 0;
}

/**
 * @brief Initializes the team variables
 * 
 * @param team Pointer to the team structure
 */
void init_team(team_t * team) {
    team->safe_cars = 0;
}

/**
 * @brief Initializes the shared memory variables
 * 
 * @param shared_memory Pointer to the shared memory structure
 */
void init_memory(shared_memory_t * shared_memory) {
    shared_memory->finish_cars = 0;
    shared_memory->race_started = 0;
    shared_memory->end_race = 0;
    shared_memory->restarting_race = 0;
    shared_memory->ranking = 0;
    shared_memory->waiting_for_reset = 0;
}

/**
 * @brief Waits for the race to start and also checks for a
 * race interruption before starting.
 * 
 * @param shared_memory Pointer to the shared memory structure
 * @return int 1 if there was an interruption or 0 otherwise
 */
int wait_for_start(shared_memory_t * shared_memory) {
    pthread_mutex_lock(&shared_memory->mutex);

    while (shared_memory->race_started == 0 || shared_memory->end_race == 1 || shared_memory->restarting_race == 1) {
        pthread_cond_wait(&shared_memory->new_command, &shared_memory->mutex);

        if(shared_memory->race_started == 0 && shared_memory->end_race == 1) {
            pthread_mutex_unlock(&shared_memory->mutex);
            return 1;
        }
    }
    
    pthread_mutex_unlock(&shared_memory->mutex);

    return 0;
}

/**
 * @brief Gets the pointer for the teams array
 * 
 * @param shared_memory Pointer to the shared memory structure
 * @return team_t* Pointer to the first team structure in the array
 */
team_t * get_teams(shared_memory_t * shared_memory) {
    return (team_t *) (shared_memory + 1);
}

/**
 * @brief Gets the pointer for the cars array
 * 
 * @param shared_memory Pointer to the shared memory structure
 * @param config Pointer to the config structure
 * @return car_t* Pointer to the first car structure in the array
 */
car_t * get_cars(shared_memory_t * shared_memory, config_t * config) {
    return (car_t *) (get_teams(shared_memory) + config->teams);
}

/**
 * @brief Gets the pointer to a certain car structure according to the team and car position received
 * 
 * @param shared_memory Pointer to the shared memory structure
 * @param config Pointer to the config structure
 * @param pos_team Team position in the array
 * @param pos_car Car position in the array
 * @return car_t* Pointer to the car structure
 */
car_t * get_car(shared_memory_t * shared_memory, config_t * config, int pos_team, int pos_car) {
    return (car_t *) (get_cars(shared_memory, config) + pos_team * config->max_cars_per_team + pos_car);
}