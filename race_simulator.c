/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "race_simulator.h"

shared_memory_t * shared_memory;
config_t * config;
key_t shmkey;
int shmid;



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
    init_mutex_log();
    
    shmid = shmget(shmkey, sizeof(shared_memory_t) + sizeof(team_t) * config->teams + sizeof(car_t) * config->max_cars_per_team * config->teams, IPC_CREAT|IPC_EXCL|0700);
    if(shmid < 1) {
        perror("Error with shmget: ");
        exit(1);
    }

    shared_memory = (shared_memory_t *) shmat(shmid, NULL, 0);
    if(shared_memory < (shared_memory_t *) 1) {
        perror("Error with shmat: ");
        exit(1);
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


    shmdt(shared_memory);
    shmctl(shmid, IPC_RMID, NULL);

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
    config = load_config();
    if(config == NULL) return -1;

    

    pid_t race_manager_pid;

    init();

    race_manager_pid = fork();
    if(race_manager_pid == 0) {
        race_manager(shared_memory, config);
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
        write_log("DEBUG: Race manager is leaving [%d]\n", race_manager_pid);
    #endif

    wait(NULL);
    waitpid(malfunction_manager_pid, NULL, 0);
    #ifdef DEBUG
        write_log("DEBUG: Malfunction manager is leaving [%d]\n", malfunction_manager_pid);
    #endif
    
    clean();

    return 0;
}