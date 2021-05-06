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

pid_t main_pid;



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
    main_pid = getpid();
    init_mutex_log();
    unlink(PIPE_NAME);
    if((mkfifo(PIPE_NAME, O_CREAT | O_EXCL | 0600) < 0) && (errno != EEXIST)) {
        perror("Cannot create pipe: ");
        exit(1);
    }
    
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

    shared_memory->message_queue = msgget(IPC_PRIVATE, IPC_CREAT|0777);
    if(shared_memory->message_queue <0){
        perror("Creating message queue");
        exit(0);
    }
    shared_memory->num_teams = 0;
    shared_memory->race_started = 0;
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
    while(wait(NULL) != -1);

    if(main_pid == getpid()) {
        write_log("\nProgram shutdown...\n");
        destroy_mutex_log();

        //pthread_cond_broadcast(&shared_memory->new_command);
        //pthread_mutex_unlock(&shared_memory->mutex);

        shmdt(shared_memory);
        shmctl(shmid, IPC_RMID, NULL);
    } else {
        shared_memory->race_started = 1;
        pthread_cond_broadcast(&shared_memory->new_command);
    }

    exit(0);
}

void show_statistics() {
    if(main_pid == getpid()) {
        write_log("\n");
        pthread_mutex_lock(&shared_memory->mutex);
        if(shared_memory->race_started== 0) {
            write_log("STATISTICS: RACE NOT STARTED\n");
            pthread_mutex_unlock(&shared_memory->mutex);
        } else {
            int max_statistics = (shared_memory->num_teams >= TOP_STATISTICS) ? TOP_STATISTICS : shared_memory->num_teams;
            car_t best_cars[max_statistics];
            for(int i = 0; i < max_statistics; i++) best_cars[i].distance = -1;
            team_t * teams = get_teams(shared_memory);
            car_t * car;
            for(int i = 0; i < shared_memory->num_teams; i++) {
                for(int j = 0; j < teams[i].num_cars; j++) {
                    car = get_car(shared_memory, config, i, j);
                    int l;
                    for(l = 0; l < max_statistics && car->distance < best_cars[l].distance; l++);
                    if(l == max_statistics) continue;
                    for(int k = max_statistics - 1; k > l; k--) best_cars[k] = best_cars[k-1];
                    best_cars[l] = *car;
                }
            }
            pthread_mutex_unlock(&shared_memory->mutex);

            write_log("STATISTICS:\n");
            write_log("| RANK | CAR | TEAM | LAPS | STOPS |\n");
            for(int i = 0; i< max_statistics; i++){
                write_log("| %4d | %3d | %4s | %4d | %5d |\n", (i+1), best_cars[i].number, best_cars[i].team->name, (int)(best_cars[i].distance/config->lap_distance), best_cars[i].total_boxstops);
            }
        }
    }
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

    signal(SIGINT, clean);
    signal(SIGTSTP, SIG_IGN);

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
        malfunction_manager(shared_memory, config);
        exit(0);
    }

    signal(SIGTSTP, show_statistics);

    /*waitpid(race_manager_pid, NULL, 0);
    #ifdef DEBUG
        write_log("DEBUG: Race manager is leaving [%d]\n", race_manager_pid);
    #endif

    waitpid(malfunction_manager_pid, NULL, 0);
    #ifdef DEBUG
        write_log("DEBUG: Malfunction manager is leaving [%d]\n", malfunction_manager_pid);
    #endif*/
    
    wait(NULL);
    wait(NULL);

    return 0;
}