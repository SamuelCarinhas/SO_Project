#include "read_config.h"
#include "functions.h"

config_t * load_config() {
    FILE * file = fopen("config/config.txt", "r");

    if(file == NULL) {
        printf("Couldn't open config file.\n");
        return NULL;
    }

    char lines[7][MAX_STRING];

    int result;

    config_t * config = (config_t *) malloc(sizeof(config_t));

    for(int i = 0; i < 7; i++) {
        result = read_line(file, lines[i], MAX_STRING);
        
        if(result == -1) {
            printf("Invalid config file.\n");
            free(config);
            return NULL;
        }
    }

    config->time_units_per_second = atoi(lines[0]);

    char * token;
    const char delim[3] = ", ";
    token = strtok(lines[1], delim);

    if(token == NULL) {
        printf("Invalid config file.\n");
        free(config);
        return NULL;
    }

    config->lap_distance = atoi(token);
    token = strtok(NULL, delim);
    
    if(token == NULL) {
        printf("Invalid config file.\n");
        free(config);
        return NULL;
    }

    config->laps = atoi(token);
    config->teams = atoi(lines[2]);

    if(config->teams < MIN_TEAMS) {
        // TODO:
        printf("BRUH, DECIDIR DPS.\n");
        free(config);
        return NULL;
    }

    config->max_cars_per_team = atoi(lines[3]);
    config->failure_time_units = atoi(lines[4]);

    token = strtok(lines[5], delim);

    if(token == NULL) {
        printf("Invalid config file.\n");
        free(config);
        return NULL;
    }

    config->min_repair_time = atoi(token);
    token = strtok(NULL, delim);
    
    if(token == NULL) {
        printf("Invalid config file.\n");
        free(config);
        return NULL;
    }
    
    config->max_repair_time = atoi(token);
    config->fuel_capacity = atoi(lines[6]);
    
    return config;
}