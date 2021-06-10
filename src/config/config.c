/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/
#include "config.h"

/**
 * @brief Load and validate values from config file
 *
 * @return config_t* Structure with the program config
 * (NULL pointer if config is invalid)
 */
config_t * load_config() {
    
    FILE * file = fopen("config/config", "r");

    if(file == NULL) {
        write_log("ERROR: Couldn't open config file.\n");
        return NULL;
    }

    char lines[7][MAX_STRING];

    int result;

    config_t * config = (config_t *) malloc(sizeof(config_t));

    for(int i = 0; i < 7; i++) {
        result = read_line(file, lines[i], MAX_STRING);
        
        if(result == -1) {
            write_log("ERROR: Invalid config file.\n");
            free(config);
            return NULL;
        }
    }

    if(!is_number(lines[0]) || !is_number(lines[2]) || !is_number(lines[3]) || !is_number(lines[4]) || !is_number(lines[6])) {
        write_log("ERROR: Invalid config file.\n");
        free(config);
        return NULL;
    }
    config->time_units_per_second = atoi(lines[0]);
    if(config->time_units_per_second > MAX_TIME_UNITS_PER_SECOND || config->time_units_per_second <= 0) {
        write_log("ERROR: Invalid config file: Time units per second must be between %d and %d\n", 1, MAX_TIME_UNITS_PER_SECOND);
        free(config);
        return NULL;
    }

    char * token;
    const char delim[3] = ", ";
    token = strtok(lines[1], delim);

    if(token == NULL && !is_number(token)) {
        write_log("ERROR: Invalid config file.\n");
        free(config);
        return NULL;
    }
    config->lap_distance = atoi(token);
    if(config->lap_distance <= 0){
        write_log("ERROR: Lap distance must be positive\n");
        free(config);
        return NULL;
    }
    token = strtok(NULL, delim);
    
    if(token == NULL && !is_number(token)) {
        write_log("ERROR: Invalid config file.\n");
        free(config);
        return NULL;
    }

    config->laps = atoi(token);
    if(config->laps <= 0){
        write_log("ERROR: Number of laps must be positive\n");
        free(config);
        return NULL;
    }
    config->teams = atoi(lines[2]);

    if(config->teams < MIN_TEAMS) {
        write_log("ERROR: The minimum teams cannot be less than 3\n");
        free(config);
        return NULL;
    }

    config->max_cars_per_team = atoi(lines[3]);
    if(config->max_cars_per_team <= 0) {
        write_log("ERROR: Invalid config file: Max cars per team must be greater than 0\n");
        free(config);
        return NULL;
    }
    config->malfunction_time_units = atoi(lines[4]);
    
    if(config->malfunction_time_units <= 0) {
        write_log("ERROR: Invalid config file: Max repair time must be greater than 0\n");
        free(config);
        return NULL;
    }

    token = strtok(lines[5], delim);

    if(token == NULL && !is_number(token)) {
        write_log("ERROR: Invalid config file.\n");
        free(config);
        return NULL;
    }

    config->min_repair_time = atoi(token);
    if(config->min_repair_time <= 0) {
        write_log("ERROR: Invalid config file: Min repair time must be greater then 0\n");
        free(config);
        return NULL;
    }

    token = strtok(NULL, delim);
    
    if(token == NULL && !is_number(token)) {
        write_log("ERROR: Invalid config file.\n");
        free(config);
        return NULL;
    }
    
    config->max_repair_time = atoi(token);

    if(config->max_repair_time <= 0) {
        write_log("ERROR: Invalid config file: Max repair time must be greater then 0\n");
        free(config);
        return NULL;
    }

    if(config->max_repair_time < config->min_repair_time) {
        write_log("ERROR: Invalid config file: Max repair time cannot be less than the min repair time.\n");
        free(config);
        return NULL;
    }

    config->fuel_capacity = atoi(lines[6]);
    
    if(config->fuel_capacity <= 0) {
        write_log("ERROR: Invalid config file: Fuel capacity must be greater than 0\n");
        free(config);
        return NULL;
    }
    
    return config;
}