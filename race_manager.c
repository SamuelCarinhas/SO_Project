/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "race_manager.h"
#include "team_manager.h"

pid_t * teams_pids;
team_t ** teams;
int current_teams = 0;

team_t * create_team(char * name, int max_cars) {
    for(int i = 0; i < current_teams; i++) {
        if(strcmp(teams[i]->name, name) == 0) {
            printf("TEAM ALREADY EXISTS\n");
            return teams[i];
        }
    }

    team_t * new_team = (team_t *) malloc(sizeof(team_t));
    
    new_team->name = name;
    new_team->cars = (car_t *) malloc(sizeof(car_t) * max_cars);

    printf("NEW TEAM CREATED\n");
    teams[current_teams++] = new_team;
    return new_team;
}

char * get_name(char * name) {
    char * new_name = malloc(sizeof(char) * strlen(name));
    strcpy(new_name, name);
    return new_name;
}

void race_manager(config_t * config) {
    teams_pids = (pid_t *) malloc(sizeof(pid_t) * config->teams);
    teams = (team_t **) malloc(sizeof(team_t *) * config->teams);

    char * names[config->teams];
    names[0] = get_name("Team Weee");
    names[1] = get_name("Team Bruh");
    names[2] = get_name("Team Benfica");
    names[3] = get_name("Team Porto");
    names[4] = get_name("Team Weee");

    for(int i = 0; i < config->teams; i++) {
        int prev_teams = current_teams;
        team_t * new_team = create_team(names[i], config->max_cars_per_team);
        
        if(prev_teams == current_teams) continue;

        teams_pids[i] = fork();
        if(teams_pids[i] == 0) {
            team_manager(config, new_team);
            exit(0);
        }
    }
    
    for(int i = 0; i < config->teams; i++) {
        waitpid(teams_pids[i], NULL, 0);
        printf("Team %d terminated\n", teams_pids[i]);
    }

    free(teams_pids);
    free(teams);
}