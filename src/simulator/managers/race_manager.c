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

static int load_car(char * string);
static int command_handler(char * string);
static int receive_car_messages(char * string);
static int get_team_position(char * team_name);

static void race();
static void clean_race();
static void race_sigusr1();
static void restart_race();
static void create_team(char * team_name, int pos);

/**
 * @brief Creates the new team and add the information to the
 * shared memory. After that creates the team's process.
 * 
 * @param team_name New team's name
 * @param pos Team's position in the shared memory teams array.
 */
static void create_team(char * team_name, int pos) {
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

/**
 * @brief Search for the position of the givem team in the shared memory teams array
 * 
 * @param team_name Team's name
 * @return int Index of the team in the array
 */
static int get_team_position(char * team_name) {
    int pos;
    for(pos = 0; pos < shared_memory->num_teams && strcmp(get_teams(shared_memory)[pos].name, team_name); pos++);
    if(pos == shared_memory->num_teams) {
        create_team(team_name, pos);
        shared_memory->num_teams++;
    }
    return pos;
}

/**
 * @brief Loads a new car into the team's car array
 * 
 * @param string Command received from namedpipe
 * @return int Logical value of the result of adding a new car
 * 0 -> Couldn't parse the command
 * 1 -> Invalid values or team is full
 * 1 -> Car added 
 */
static int load_car(char * string) {
    char team_name[MAX_STRING];
    int car_number, speed, reliability;
    double consumption;
    
    const char * format = "ADDCAR TEAM: %[a-zA-Z0-9 ], CAR: %d, SPEED: %d, CONSUMPTION: %lf, RELIABILITY: %d";

    int res = sscanf(string, format, team_name, &car_number, &speed, &consumption, &reliability);

    if(res != 5)
        return 0;

    if(strlen(team_name) < MIN_TEAM_NAME || strlen(team_name) > MAX_TEAM_NAME) {
        write_log("TEAM NAME MUST BE BETWEEN %d AND %d\n", MIN_TEAM_NAME, MAX_TEAM_NAME);
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
    init_mutex_proc(&car->car_mutex);
    init_car(car, config);

    shared_memory->total_cars++;
    team->num_cars++;

    return 1;
}

/**
 * @brief Handles the SIGUSR1 signal
 * If its possible to restart the race, notify the program that
 * the race needs to restart.
 * 
 */
static void race_sigusr1() {
    write_log("RACE MANAGER: SIGUSR1\n");

    pthread_mutex_lock(&shared_memory->mutex);
    if(shared_memory->race_started == 0 || shared_memory->end_race) {
        pthread_mutex_unlock(&shared_memory->mutex);
        write_log("CANNOT RESTART RACE\n");
        return;
    }
    shared_memory->end_race = 1;
    shared_memory->restarting_race = 1;
    pthread_mutex_unlock(&shared_memory->mutex);
}

/**
 * @brief Sends a signal to every team that they need to restart and
 * wait for every team and the malfunction receive that information.
 * After that, reset shared memory variables to their default values.
 * 
 */
static void restart_race() {
    team_t * teams = get_teams(shared_memory);
    for(int i = 0; i < shared_memory->num_teams; i++) {
        pthread_mutex_lock(&teams[i].team_mutex);
        teams[i].box.request_reservation = 1;
        pthread_mutex_unlock(&teams[i].team_mutex);
        pthread_cond_signal(&teams[i].box.request);
    }

    show_statistics(shared_memory, config, 0);

    for(int i = 0; i < shared_memory->num_teams; i++) {
        team_t * team = &teams[i];
        init_team(team);
        for(int j = 0; j < team->num_cars; j++) {
            init_car(get_car(shared_memory, config, team->pos_array, j), config);
        }
    }
    
    pthread_mutex_lock(&shared_memory->mutex);
    int num_teams = shared_memory->num_teams;
    pthread_mutex_unlock(&shared_memory->mutex);

    // Wait for every process has received the information that the race is restarting
    pthread_mutex_lock(&shared_memory->mutex_reset);
    while(shared_memory->waiting_for_reset < num_teams + 1)
        pthread_cond_wait(&shared_memory->reset_race, &shared_memory->mutex_reset);
    pthread_mutex_unlock(&shared_memory->mutex_reset);
    init_memory(shared_memory);

    write_log("RACE WAS RESETED\n");
}

/**
 * @brief Close the teams pipes and frees the pipes array
 * 
 */
static void clean_race() {
    close(fd);
    for(int i = 0; i< shared_memory->num_teams; i++){
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for(int i = 0; i < config->teams; i++) {
        free(pipes[i]);
    }

    free(pipes);
    free(teams_pids);
} 

/**
 * @brief Handles the command received from named pipe
 * 
 * @param string Command
 * @return int Logical value of the command parsing
 * OK -> Keep receiving commands from pipe
 * END -> Can stop receiving commands from pipe
 */
static int command_handler(char * string) {
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

/**
 * @brief Handles the cars messages received from unnamed pipe
 * 
 * @param string Car message
 * @return int Logical value of the message parsing
 * OK -> Keep reading car messages
 * END -> Stop reading from the unnamed pipes
 */
static int receive_car_messages(char * string) {
    write_log("%s\n", string);
    if(ends_with(string, "FINISHED THE RACE") || ends_with(string, "GAVE UP")) {
        pthread_mutex_lock(&shared_memory->mutex);
        shared_memory->finish_cars++;
        if(shared_memory->finish_cars == shared_memory->total_cars) {
            pthread_mutex_unlock(&shared_memory->mutex);
            write_log("RACE FINISHED\n");
            return END;
        }
        pthread_mutex_unlock(&shared_memory->mutex);
    }
    return OK;
}

/**
 * @brief Loop to handle the race, read commands from named pipe
 * and the received messages from cars
 * 
 */
static void race() {
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    while(1) {
        signal(SIGUSR1, race_sigusr1);
        read_from_pipes(shared_memory, &fd, 1, NULL, command_handler);

        pthread_mutex_lock(&shared_memory->mutex);
        int race_started = shared_memory->race_started;
        pthread_mutex_unlock(&shared_memory->mutex);

        if(race_started == 0)
            return;

        int pipe_arr[shared_memory->num_teams + 1];
        pipe_arr[0] = fd;
        for(int i = 1; i <= shared_memory->num_teams; i++)
            pipe_arr[i] = pipes[i-1][0];
        
        read_from_pipes(shared_memory, pipe_arr, shared_memory->num_teams + 1, receive_car_messages, command_handler);
        
        pthread_mutex_lock(&shared_memory->mutex);
        int restarting_race = shared_memory->restarting_race;
        pthread_mutex_unlock(&shared_memory->mutex);
        if(restarting_race) {
            restart_race();
        } else {
            break;
        }
    }
}

/**
 * @brief Creates the unnamed pipes and calls the race function.
 * After the race is ended displays the statistics and frees the
 * allocated resources by calling the clean_race function.
 * 
 * @param shared Pointer to the shared memory structure
 * @param conf Pointer to the config structure
 */
void race_manager(shared_memory_t * shared, config_t * conf) {
    shared_memory = shared;
    config = conf;

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

    race();

    pthread_mutex_lock(&shared_memory->mutex);
    int race_started = shared_memory->race_started;
    shared_memory->race_started = 0;
    pthread_mutex_unlock(&shared_memory->mutex);

    team_t * teams = get_teams(shared_memory);
    for(int i = 0; i < shared_memory->num_teams; i++) {
        pthread_mutex_lock(&teams[i].team_mutex);
        teams[i].box.request_reservation = 1;
        pthread_mutex_unlock(&teams[i].team_mutex);
        pthread_cond_signal(&teams[i].box.request);
    }

    for(int i = 0; i < shared_memory->num_teams; i++) {
        waitpid(teams_pids[i], NULL, 0);
        write_debug("TEAM %s IS LEAVING [%d]\n", get_teams(shared_memory)[i].name, teams_pids[i]);
    }

    show_statistics(shared_memory, config, race_started);
    clean_race();
    
    write_debug("RACE MANAGER LEFT\n");
}