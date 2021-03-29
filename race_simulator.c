/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "read_config.h"
#include "race_manager.h"
#include "malfunction_manager.h"
#include "functions.h"
#include "team_manager.h"

team_t * teams;
config_t * config;
int shmid_teams, * shmid_cars;

void init(){
    if ((shmid_teams = shmget(IPC_PRIVATE, sizeof(team_t) * config->teams, IPC_CREAT | 0700)) < 0) {
		perror("Error in shmget with IPC_CREAT\n");
		exit(1);
	}

	if ((teams = ((team_t *) shmat(shmid_teams, NULL, 0))) == ((team_t*)-1)) {
		perror("Shmat error!");
		exit(1);
	}


    shmid_cars = (int *) malloc(sizeof(int) * config->max_cars_per_team);
    for(int i = 0; i < config->teams; i++) {
        if ((shmid_cars[i] = shmget(IPC_PRIVATE, sizeof(car_t) * config->max_cars_per_team, IPC_CREAT | 0700)) < 0) {
            perror("Error in shmget with IPC_CREAT\n");
            exit(1);
        }

        if ((teams[i].cars = ((car_t *) shmat(shmid_cars[i], NULL, 0))) == ((car_t*)-1)) {
            perror("Shmat error!");
            exit(1);
        }
    }
    
    init_mutex_log();

    write_log("SIMULATOR STARTING");
}

void clean() {
    destroy_mutex_log();

    for(int i = 0; i < config->teams; i++) {
        shmdt(teams[i].cars);
        shmctl(shmid_cars[i], IPC_RMID, NULL);
    }

    shmdt(teams);
	shmctl(shmid_teams, IPC_RMID, NULL);

    free(shmid_cars);
}

int main() {

    config = load_config();
    if(config == NULL) return -1;

    pid_t race_manager_pid;

    init();

    race_manager_pid = fork();
    if(race_manager_pid == 0) {
        race_manager(config, teams);
        exit(0);
    }

    pid_t malfunction_manager_pid;

    malfunction_manager_pid = fork();
    if(malfunction_manager_pid == 0) {
        malfunction_manager(config);
        exit(0);
    }

    waitpid(race_manager_pid, NULL, 0);
    waitpid(malfunction_manager_pid, NULL, 0);
    
    clean();

    return 0;
}