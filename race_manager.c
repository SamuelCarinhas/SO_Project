/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "race_manager.h"

pid_t * teams_pids;
int current_teams = 0;

void race_manager(config_t * config, team_t * teams) {
    teams_pids = (pid_t *) malloc(sizeof(pid_t) * config->teams);

    for(int i = 0; i < 2; i++) {
        if(i == 0) strcpy(teams[i].name, "Weeeeeeee");
        else strcpy(teams[i].name, "iiiiiiiiii");
        
        teams_pids[i] = fork();
        if(teams_pids[i] == 0) {
            team_manager(config, &teams[i]);
            exit(0);
        }
    }
    
    for(int i = 0; i < config->teams; i++) {
        waitpid(teams_pids[i], NULL, 0);
    }

    free(teams_pids);
}