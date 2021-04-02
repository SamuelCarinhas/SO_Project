/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "race_manager.h"

pid_t * teams_pids;
int current_teams = 0;

void create_team(char * team_name, shared_memory_t * shared_memory, int pos) {
    strcpy(shared_memory->teams[pos].name, team_name);

    pthread_condattr_t attrcondv;
    pthread_condattr_init(&attrcondv);
    pthread_condattr_setpshared(&attrcondv, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&shared_memory->teams[pos].new_command, &attrcondv);
    shared_memory->teams[pos].num_cars = 0;
        
    teams_pids[pos] = fork();
    if(teams_pids[pos] == 0) {
        printf("I am a processes with id %d\n", getpid());
        //team_manager(config, &teams[i]);
        exit(0);
    }
}
        

int get_team_position(char * team_name, shared_memory_t * shared_memory) {
    int pos;
    for(pos = 0; pos < shared_memory->num_teams && strcmp(shared_memory->teams[pos].name, team_name); pos++);
    if(pos == shared_memory->num_teams) {
        create_team(team_name, shared_memory, pos);
        shared_memory->num_teams++;
    }
    return pos;
}

team_t * load_team(char * string, shared_memory_t * shared_memory) {
    char delim[2] = ",";
    char delim_b[2] = ":";
    char * token, * token_b;
    char * rest = string, * rest_b;
    char data[5][MAX_STRING];
    for(int i = 0; i < 5; i++) {
        token = strtok_r(rest, delim, &rest);
        rest_b = token;
        token_b = strtok_r(rest_b, delim_b, &rest_b);
        token_b = strtok_r(rest_b, delim_b, &rest_b);
        strcpy(data[i], trim(token_b));
    }

    int pos_team = get_team_position(data[0], shared_memory);
    int pos_car = shared_memory->teams[pos_team].num_cars;
    strcpy(shared_memory->teams[pos_team].cars[pos_car].team_name, data[0]);
    shared_memory->teams[pos_team].cars[pos_car].number = atoi(data[1]);
    shared_memory->teams[pos_team].cars[pos_car].speed = atoi(data[2]);
    shared_memory->teams[pos_team].cars[pos_car].consuption = atof(data[3]); 
    shared_memory->teams[pos_team].cars[pos_car].reliability = atoi(data[4]); 
    shared_memory->teams[pos_team].num_cars++;
    
    
    /*printf("%s %d %d %.2f %d\n",shared_memory->teams[pos_team].cars[pos_car].team_name,
    shared_memory->teams[pos_team].cars[pos_car].number,
    shared_memory->teams[pos_team].cars[pos_car].speed,
    shared_memory->teams[pos_team].cars[pos_car].consuption,
    shared_memory->teams[pos_team].cars[pos_car].reliability
    );*/

    return &shared_memory->teams[pos_team];
}

void race_manager(shared_memory_t * shared_memory) {
    teams_pids = (pid_t *) malloc(sizeof(pid_t) * shared_memory->config->teams);

    char string[MAX_STRING];
    while(1) {
        read_line(stdin, string, MAX_STRING);
        if(starts_with(string, "ADDCAR")) {
            team_t * team = load_team(string, shared_memory);
            pthread_cond_signal(&team->new_command);
        } else if (starts_with(string, "START RACE!")) {
            shared_memory->race_started = 1;
            for(int i = 0; i < shared_memory->num_teams; i++)
                pthread_cond_signal(&shared_memory->teams[i].new_command);
        }
        
    }
    
    load_team(string, shared_memory);
    
    
    for(int i = 0; i < shared_memory->num_teams; i++) {
        waitpid(teams_pids[i], NULL, 0);
        printf("KILL %d\n", teams_pids[i]);
    }

    free(teams_pids);
}