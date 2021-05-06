/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "race_manager.h"

shared_memory_t * shared_memory;
config_t * config;
pid_t * teams_pids;
int ** pipes;
int fd;

/*
* NAME :                            void create_team(char * team_name, shared_memory_t * shared_memory, int pos)
*
* DESCRIPTION :                     Adds the new team's information to the shared memory and creates his process
*
* PARAMETERS :
*           char *                  team_name              new team's name
*           shared_memory_t *       shared_memory          pointer to the shared memory
*           int                     pos                    position where to insert the new team
*       
* OUTPUTS :
*       void
*
*/
void create_team(char * team_name, int pos) {
    team_t * team = get_teams(shared_memory) + pos;
    strcpy(team->name, team_name);
    team->num_cars = 0;
    team->res = 0;
    team->pos_array = pos;
    pthread_mutexattr_t attrmutex;
    pthread_mutexattr_init(&attrmutex);
    pthread_mutexattr_setpshared(&attrmutex, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&team->team_mutex, &attrmutex);

    pipe(pipes[pos]);

    teams_pids[pos] = fork();
    if(teams_pids[pos] == 0) {
        close(pipes[pos][0]);
        team_manager(shared_memory, team, config, pipes[pos][1]);
        exit(0);
    }else
        close(pipes[pos][1]);
}

/*
* NAME :                            int get_team_position(char * team_name, shared_memory_t * shared_memory)
*
* DESCRIPTION :                     Search for the position of the given team in the shared_memory teams array
*
* PARAMETERS :
*           char *                  team_name              team's name
*           shared_memory_t *       shared_memory          pointer to the shared memory
*       
* RETURN :
*           int                     Index of the team in the array 
*
* TODO :                            Check if the limit of teams wasn't exceeded
*
*/
int get_team_position(char * team_name) {
    int pos;
    for(pos = 0; pos < shared_memory->num_teams && strcmp(get_teams(shared_memory)[pos].name, team_name); pos++);
    if(pos == shared_memory->num_teams) {
        create_team(team_name, pos);
        shared_memory->num_teams++;
    }
    return pos;
}


/*
* NAME :                            team_t * load_car(char * string, shared_memory_t * shared_memory)
*
* DESCRIPTION :                     Loads a new car into the team's car array
*
* PARAMETERS :
*           char *                  string                 NAMEDPIPE command
*           shared_memory_t *       shared_memory          pointer to the shared memory
*       
* RETURN :
*           team_t                  pointer to the modified team
*
* TODO :                            Check the limit of cars per team and make sure the command is valid
*
*/
team_t * load_car(char * string) {
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

    int pos_team = get_team_position(data[0]);

    team_t * team = get_teams(shared_memory) + pos_team;
    
    int pos_car = team->num_cars;
    car_t * car = get_car(shared_memory, config, pos_team, pos_car);
    car->team = team;
    car->number = atoi(data[1]);
    car->speed = atoi(data[2]);
    car->current_speed = car->speed;
    car->consuption = atof(data[3]);
    car->reliability = atoi(data[4]);
    car->fuel = config->fuel_capacity;
    car->distance = 0;
    team->num_cars++;
    car->total_malfunctions = 0;
    car->total_refuels = 0;
    car->total_boxstops = 0;

    return team;
}

void clean_race() {
    for(int i = 0; i < shared_memory->num_teams; i++) {
        waitpid(teams_pids[i], NULL, 0);
        #ifdef DEBUG
            write_log("DEBUG: TEAM %s IS LEAVING [%d]\n", get_teams(shared_memory)[i].name, teams_pids[i]);
        #endif
    }
    
    close(fd);
    team_t * teams = get_teams(shared_memory);
    for(int i = 0; i< shared_memory->num_teams; i++){
        pthread_mutex_destroy(&teams[i].team_mutex);
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for(int i = 0; i < config->teams; i++) {
        free(pipes[i]);
    }

    free(pipes);
    free(teams_pids);
}

/*
* NAME :                            void race_manager(shared_memory_t * shared_memory)
*
* DESCRIPTION :                     Function to handle the Race Manager process
*
* PARAMETERS :
*           shared_memory_t *       shared_memory          pointer to the shared memory
*       
* RETURN :
*          void
*
* TODO :                            Handle possible errors (Creating the teams adding cars)
*
*/
void race_manager(shared_memory_t * shared, config_t * conf) {
    shared_memory = shared;
    config = conf;

    signal(SIGINT, clean_race);

    pipes = (int **) malloc(sizeof(int *) * config->teams);
    for(int i = 0; i < config->teams; i++) {
        pipes[i] = (int *) malloc(sizeof(int) * 2);
    }

    #ifdef DEBUG
        write_log("DEBUG: Race manager created [%d]\n", getpid());
    #endif

    
    fd = open(PIPE_NAME, O_RDONLY|O_NONBLOCK);
    if(fd < 0) {
        perror("Cannot open pipe for reading: ");
        exit(-1);
    }

    teams_pids = (pid_t *) malloc(sizeof(pid_t) * config->teams);

    char string[MAX_STRING];
    fd_set read_set;
    
    while(1) {
        FD_ZERO(&read_set);
        FD_SET(fd, &read_set);
        if(select(fd + 1, &read_set, NULL, NULL, NULL)>0){
            if(FD_ISSET(fd, &read_set)){
                read(fd, string, MAX_STRING);
                if(starts_with(string, "ADDCAR")) {
                    load_car(string);
                    pthread_cond_broadcast(&shared_memory->new_command);
                } else if (starts_with(string, "START RACE!")) {
                    write_log("NEW COMMAND RECEIVED: START RACE \n");
                    //!VERIFICAR SE TEM EQUIPAS SUFICIENTES!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                    shared_memory->race_started = 1;
                    pthread_cond_broadcast(&shared_memory->new_command);
                    break;
                }
            }
            close(fd);
            fd = open(PIPE_NAME, O_RDONLY|O_NONBLOCK);
        }
    }

    while(1) {
        FD_ZERO(&read_set);
        for(int i = 0; i < shared_memory->num_teams; i++)
            FD_SET(pipes[i][0], &read_set);
        if(select(pipes[shared_memory->num_teams-1][0] + 1, &read_set, NULL, NULL, NULL)>0){
            for(int i = 0; i < shared_memory->num_teams; i++){
                if(FD_ISSET(pipes[i][0], &read_set)){
                    read(pipes[i][0], string, MAX_STRING);
                    write_log("|| %s\n", string);
                }
            }
        }
    }

    clean_race();
}