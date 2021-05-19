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
pid_t malfunction_manager_pid;
int ** pipes;
int fd;

int command_handler(char * string);
int receive_car_messages(char * string);

void race();
void clean_race();

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
* RETURN :
*       void
*
*/
void create_team(char * team_name, int pos) {
    team_t * team = get_teams(shared_memory) + pos;
    strcpy(team->name, team_name);
    team->num_cars = 0;
    team->res = 0;
    team->pos_array = pos;
    init_team(team);
    init_mutex_proc(&team->team_mutex);

    pipe(pipes[pos]);

    teams_pids[pos] = fork();
    if(teams_pids[pos] == 0) {
        close(pipes[pos][0]);
        team_manager(shared_memory, team, config, pipes[pos][1]);
        exit(0);
    } else
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
int load_car(char * string) {
    char team_name[MAX_STRING];
    int car_number, speed, reliability;
    double consumption;
    
    const char * format = "ADDCAR TEAM: %[a-zA-Z0-9 ], CAR: %d, SPEED: %d, CONSUMPTION: %lf, RELIABILITY: %d";

    int res = sscanf(string, format, team_name, &car_number, &speed, &consumption, &reliability);

    if(res != 5)
        return 0;

    if(strlen(team_name) < 3 || strlen(team_name) > 16) {
        write_log("TEAM NAME MUST BE BETWEEN 3 AND 10\n");
        return -1;
    }
    
    team_t * teams = get_teams(shared_memory);
    int car_exists = 0;
    for(int i = 0; i < shared_memory->num_teams && !car_exists; i++) {
        for(int j = 0; j< teams[i].num_cars ; j++)
            if (car_number == get_car(shared_memory, config, i, j)->number) {
                car_exists = 1;
                break;
            }
    }

    if(car_exists){
        write_log("DUPLICATE CAR NUMBER\n");
        return -1;
    }

    if(speed <= 0) {
        write_log("INVALID SPEED VALUE\n");
        return -1;
    }

    if(reliability > 100 || reliability <= 0) {
        write_log("INVALID RELIABILITY VALUE\n");
        return -1;
    }

    if(consumption <= 0) {
        write_log("INVALID CONSUMPTION VALUE\n");
        return -1;
    }

    int pos_team = get_team_position(team_name);

    team_t * team = &teams[pos_team];

    if(team->num_cars == config->max_cars_per_team) {
        write_log("TEAM %s CAN'T HAVE MORE CARS [MAX OF %d CARS]\n", team->name, config->max_cars_per_team);
        return -1;
    }

    int pos_car = team->num_cars;
    car_t * car = get_car(shared_memory, config, pos_team, pos_car);
    car->team = team;
    car->number =car_number;
    
    car->speed = speed;
    car->consumption = consumption;
    car->reliability = reliability;

    init_car(car, config);

    shared_memory->total_cars++;
    team->num_cars++;

    return 1;
}

void reset_race() {
    pthread_mutex_lock(&shared_memory->mutex);
    if(shared_memory->race_started == 0) {
        pthread_mutex_unlock(&shared_memory->mutex);
        write_log("CANNOT RESTART RACE\n");
        return;
    }
    shared_memory->end_race = 1;
    shared_memory->restarting_race = 1;
    pthread_mutex_unlock(&shared_memory->mutex);
    
    int pipe_arr[shared_memory->num_teams + 1];
    pipe_arr[0] = fd;
    for(int i = 1; i <= shared_memory->num_teams; i++)
        pipe_arr[i] = pipes[i-1][0];
    //fica a espera que todos os carros terminem
    read_from_pipes(pipe_arr, shared_memory->num_teams + 1, receive_car_messages, command_handler);

    show_statistics(shared_memory, config);

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!! FAZER O SIGUSR1 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    team_t * teams = get_teams(shared_memory);
    for(int i = 0; i < shared_memory->num_teams; i++) {
        team_t * team = &teams[i];
        init_team(team);
        for(int j = 0; j < team->num_cars; j++) {
            init_car(get_car(shared_memory, config, team->pos_array, j), config);
        }
    }

    // ESPERA PELAS THREADS

    init_memory(shared_memory);

    write_log("RACE WAS RESTARTED\n");
}

void clean_race() {
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

int command_handler(char * string) {
    pthread_mutex_lock(&shared_memory->mutex);
    if(shared_memory->race_started == 0) {
        pthread_mutex_unlock(&shared_memory->mutex);
        if(starts_with(string, "ADDCAR")) {
            int res = load_car(string);
            if(res == 1)
                pthread_cond_broadcast(&shared_memory->new_command);
            else if(res == 0)
                write_log("WRONG COMMAND => %s\n", string);
        } else if (starts_with(string, "START RACE!")) {
            write_log("NEW COMMAND RECEIVED: START RACE\n");
            pthread_mutex_lock(&shared_memory->mutex);
            if(shared_memory->num_teams < MIN_TEAMS) {
                pthread_mutex_unlock(&shared_memory->mutex);
                write_log("CANNOT START, NOT ENOUGH TEAMS\n");
                return OK;
            }
            shared_memory->race_started = 1;
            pthread_mutex_unlock(&shared_memory->mutex);
            pthread_cond_broadcast(&shared_memory->new_command);
            return END;
        } else
            write_log("WRONG COMMAND => %s\n", string);
    } else {
        pthread_mutex_unlock(&shared_memory->mutex);
        write_log("\"%s\" : REJECTED, RACE ALREADY STARTED!\n", string);
    }
    return OK;
}

int receive_car_messages(char * string) {
    write_log("%s\n", string);
    if(ends_with(string, "FINISHED THE RACE") || ends_with(string, "GAVE UP")) {
        pthread_mutex_lock(&shared_memory->mutex);
        if(shared_memory->finish_cars == shared_memory->total_cars) {
            pthread_mutex_unlock(&shared_memory->mutex);
            write_log("RACE FINISHED\n");

            pthread_mutex_lock(&shared_memory->mutex);
            shared_memory->race_started = 0;
            pthread_mutex_unlock(&shared_memory->mutex);

            team_t * teams = get_teams(shared_memory);
            for(int i = 0; i < shared_memory->num_teams; i++) {
                pthread_mutex_lock(&teams[i].team_mutex);
                teams[i].box.request_reservation = 1;
                pthread_mutex_unlock(&teams[i].team_mutex);
                pthread_cond_signal(&teams[i].box.request);
            }

            return END;
        }
        pthread_mutex_unlock(&shared_memory->mutex);
    } else if(ends_with(string, "FINISH"))
        return FINISH;

    return OK;
}

void race() {
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGUSR1, reset_race);

    read_from_pipes(&fd, 1, NULL, command_handler);

    int pipe_arr[shared_memory->num_teams + 1];
    pipe_arr[0] = fd;
    for(int i = 1; i <= shared_memory->num_teams; i++)
        pipe_arr[i] = pipes[i-1][0];
    
    read_from_pipes(pipe_arr, shared_memory->num_teams + 1, receive_car_messages, command_handler);
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
void race_manager(shared_memory_t * shared, config_t * conf, pid_t malfunction) {
    shared_memory = shared;
    config = conf;
    malfunction_manager_pid = malfunction;


    pipes = (int **) malloc(sizeof(int *) * config->teams);
    for(int i = 0; i < config->teams; i++)
        pipes[i] = (int *) malloc(sizeof(int) * 2);

    write_debug("RACE MANAGER CREATED [%d]\n", getpid());
    
    fd = open(PIPE_NAME, O_RDONLY|O_NONBLOCK);
    if(fd < 0) {
        perror("Cannot open pipe for reading: ");
        exit(-1);
    }

    teams_pids = (pid_t *) malloc(sizeof(pid_t) * config->teams);

    //read comments from named & unnamed pipes
    race();

    for(int i = 0; i < shared_memory->num_teams; i++) {
        waitpid(teams_pids[i], NULL, 0);
        write_debug("TEAM %s IS LEAVING [%d]\n", get_teams(shared_memory)[i].name, teams_pids[i]);
    }

    show_statistics(shared_memory, config);
    clean_race();
    
    write_debug("RACE MANAGER LEFT\n");
}