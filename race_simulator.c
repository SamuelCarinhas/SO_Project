/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "race_simulator.h"

shared_memory_t * shared_memory;
int shmid, shmid_teams, * shmid_cars, shmid_config;

/*
* NAME :                            void pre_init()
*
* DESCRIPTION :                     Allocates space for the shared memory and for the race configurations
*
* PARAMETERS :
*          void
*       
* RETURN :
*          void
*
*/
void pre_init() {
    if ((shmid = shmget(IPC_PRIVATE, sizeof(shared_memory_t) , IPC_CREAT | 0700)) < 0) {
		write_log("Error in shmget with IPC_CREAT\n");
		exit(1);
	}

	if ((shared_memory = ((shared_memory_t *) shmat(shmid, NULL, 0))) == ((shared_memory_t *) -1)) {
		write_log("Shmat error!");
		exit(1);
	}

    if ((shmid_config = shmget(IPC_PRIVATE, sizeof(config_t), IPC_CREAT | 0700)) < 0) {
		write_log("Error in shmget with IPC_CREAT\n");
		exit(1);
	}

	if ((shared_memory->config = ((config_t *) shmat(shmid_config, NULL, 0))) == ((config_t*) -1)) {
		write_log("Shmat error!");
		exit(1);
	}
    
    init_mutex_log();
}

/*
* NAME :                            void init()
*
* DESCRIPTION :                     Allocates space for teams and cars. Also inicializes mutex semaphores and condition variable
*
* PARAMETERS :
*          void
*       
* RETURN :
*          void
*
*/
void init() {
    if ((shmid_teams = shmget(IPC_PRIVATE, sizeof(team_t) * shared_memory->config->teams, IPC_CREAT | 0700)) < 0) {
		write_log("Error in shmget with IPC_CREAT\n");
		exit(1);
	}

	if ((shared_memory->teams = ((team_t *) shmat(shmid_teams, NULL, 0))) == ((team_t*) -1)) {
		write_log("Shmat error!");
		exit(1);
	}

    shmid_cars = (int *) malloc(sizeof(int) * shared_memory->config->max_cars_per_team);
    for(int i = 0; i < shared_memory->config->teams; i++) {
        if ((shmid_cars[i] = shmget(IPC_PRIVATE, sizeof(car_t) * shared_memory->config->max_cars_per_team, IPC_CREAT | 0700)) < 0) {
            write_log("Error in shmget with IPC_CREAT\n");
            exit(1);
        }

        if ((shared_memory->teams[i].cars = ((car_t *) shmat(shmid_cars[i], NULL, 0))) == ((car_t*)-1)) {
            write_log("Shmat error!");
            exit(1);
        }
    }

    pthread_mutexattr_t attrmutex;
    pthread_mutexattr_init(&attrmutex);
    pthread_mutexattr_setpshared(&attrmutex, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shared_memory->mutex, &attrmutex);

    pthread_condattr_t attrcondv;
    pthread_condattr_init(&attrcondv);
    pthread_condattr_setpshared(&attrcondv, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&shared_memory->new_command, &attrcondv);
    

    write_log("SIMULATOR STARTING\n");
}

/*
* NAME :                            void clean()
*
* DESCRIPTION :                     Free the shared memory
*
* PARAMETERS :
*          void
*       
* RETURN :
*          void
*
*/
void clean() {
    destroy_mutex_log();

    pthread_mutex_destroy(&shared_memory->mutex);
    pthread_cond_destroy(&shared_memory->new_command);
    

    for(int i = 0; i < shared_memory->config->teams; i++) {
        shmdt(shared_memory->teams[i].cars);
        shmctl(shmid_cars[i], IPC_RMID, NULL);
    }

    shmdt(shared_memory->teams);
	shmctl(shmid_teams, IPC_RMID, NULL);
    shmdt(shared_memory->config);
	shmctl(shmid_config, IPC_RMID, NULL);

    shmdt(shared_memory);
    shmctl(shmid, IPC_RMID, NULL);

    free(shmid_cars);
}

/*
* NAME :                            int main()
*
* DESCRIPTION :                     Main function
*
* PARAMETERS :
*          void
*       
* RETURN :
*          int                      0 if every thing went well
*
*/
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
        malfunction_manager(shared_memory);
        exit(0);
    }
    
    waitpid(race_manager_pid, NULL, 0);
    #ifdef DEBUG
        write_log("DEBUG: Malfunction manager is leaving [%d]\n", malfunction_manager_pid);
    #endif

    waitpid(malfunction_manager_pid, NULL, 0);
    #ifdef DEBUG
        write_log("DEBUG: Race manager is leaving [%d]\n", race_manager_pid);
    #endif
    
    clean();

    return 0;
}