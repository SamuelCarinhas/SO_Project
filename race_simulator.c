/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "race_simulator.h"

shared_memory_t * shared_memory;
int shmid, shmid_teams, * shmid_cars, shmid_config;

void pre_init() {
    if ((shmid = shmget(IPC_PRIVATE, sizeof(shared_memory_t) , IPC_CREAT | 0700)) < 0) {
		perror("Error in shmget with IPC_CREAT\n");
		exit(1);
	}

	if ((shared_memory = ((shared_memory_t *) shmat(shmid, NULL, 0))) == ((shared_memory_t *) -1)) {
		perror("Shmat error!");
		exit(1);
	}

    if ((shmid_config = shmget(IPC_PRIVATE, sizeof(config_t), IPC_CREAT | 0700)) < 0) {
		perror("Error in shmget with IPC_CREAT\n");
		exit(1);
	}

	if ((shared_memory->config = ((config_t *) shmat(shmid_config, NULL, 0))) == ((config_t*) -1)) {
		perror("Shmat error!");
		exit(1);
	}
}

void init() {
    if ((shmid_teams = shmget(IPC_PRIVATE, sizeof(team_t) * shared_memory->config->teams, IPC_CREAT | 0700)) < 0) {
		perror("Error in shmget with IPC_CREAT\n");
		exit(1);
	}

	if ((shared_memory->teams = ((team_t *) shmat(shmid_teams, NULL, 0))) == ((team_t*) -1)) {
		perror("Shmat error!");
		exit(1);
	}

    shmid_cars = (int *) malloc(sizeof(int) * shared_memory->config->max_cars_per_team);
    for(int i = 0; i < shared_memory->config->teams; i++) {
        if ((shmid_cars[i] = shmget(IPC_PRIVATE, sizeof(car_t) * shared_memory->config->max_cars_per_team, IPC_CREAT | 0700)) < 0) {
            perror("Error in shmget with IPC_CREAT\n");
            exit(1);
        }

        if ((shared_memory->teams[i].cars = ((car_t *) shmat(shmid_cars[i], NULL, 0))) == ((car_t*)-1)) {
            perror("Shmat error!");
            exit(1);
        }
    }
    
    init_mutex_log();

    write_log("SIMULATOR STARTING");
}

void clean() {
    destroy_mutex_log();

    for(int i = 0; i < shared_memory->config->teams; i++) {
        shmdt(shared_memory->teams[i].cars);
        shmctl(shmid_cars[i], IPC_RMID, NULL);
    }

    shmdt(shared_memory->teams);
	shmctl(shmid_teams, IPC_RMID, NULL);

    free(shmid_cars);
}

int main() {

    pre_init();

    shared_memory->config = load_config();
    if(shared_memory->config == NULL) return -1;

    pid_t race_manager_pid;

    init();

    race_manager_pid = fork();
    if(race_manager_pid == 0) {
        race_manager(shared_memory);
        exit(0);
    }

    pid_t malfunction_manager_pid;

    malfunction_manager_pid = fork();
    if(malfunction_manager_pid == 0) {
        malfunction_manager(shared_memory->config);
        exit(0);
    }

    waitpid(race_manager_pid, NULL, 0);
    waitpid(malfunction_manager_pid, NULL, 0);
    
    clean();

    return 0;
}