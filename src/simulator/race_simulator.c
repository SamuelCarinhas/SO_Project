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
pid_t race_manager_pid, malfunction_manager_pid, clock_pid;

void signal_tstp();
void signal_sigint();
void clean();
void init();

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

    init_mutex_proc(&shared_memory->mutex);
    init_mutex_proc(&shared_memory->mutex_reset);
    //init_mutex_proc(&shared_memory->end_race_mutex);
    //init_mutex_proc(&shared_memory->clock.wait_mutex);
    init_mutex_proc(&shared_memory->clock.mutex);
    init_cond_proc(&shared_memory->new_command);
    init_cond_proc(&shared_memory->reset_race);
    //init_cond_proc(&shared_memory->end_race_cond);
    //init_cond_proc(&shared_memory->clock.wait_cond);
    init_cond_proc(&shared_memory->clock.time_cond);

    shared_memory->message_queue = msgget(IPC_PRIVATE, IPC_CREAT|0777);
    if(shared_memory->message_queue <0){
        perror("Creating message queue");
        exit(0);
    }
    shared_memory->num_teams = 0;
    shared_memory->race_started = 0;
    init_memory(shared_memory);
    shared_memory->total_cars = 0;
    //shared_memory->clock.waiting = 0;
    write_log("SIMULATOR STARTING [%d]\n", getpid());
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
    destroy_mutex_proc(&shared_memory->mutex);
    destroy_mutex_proc(&shared_memory->mutex_reset);
    //destroy_mutex_proc(&shared_memory->end_race_mutex);
    // destroy_mutex_proc(&shared_memory->clock.wait_mutex);
    destroy_mutex_proc(&shared_memory->clock.mutex);

    destroy_cond_proc(&shared_memory->new_command);
    destroy_cond_proc(&shared_memory->reset_race);
    //destroy_cond_proc(&shared_memory->end_race_cond);
    //destroy_cond_proc(&shared_memory->clock.wait_cond);
    destroy_cond_proc(&shared_memory->clock.time_cond);
    write_log("SIMULATOR CLOSING [%d]\n", getpid());
    destroy_mutex_log();
    unlink(PIPE_NAME);
    msgctl(shared_memory->message_queue, IPC_RMID, 0);
    shmdt(shared_memory);
    shmctl(shmid, IPC_RMID, NULL);
}

void signal_sigint() {
    pthread_mutex_lock(&shared_memory->mutex);
    int reset = shared_memory->restarting_race;
    if(reset == 0)
        shared_memory->end_race = 1; 
    int race_started = shared_memory->race_started;
    pthread_mutex_unlock(&shared_memory->mutex);

    if(reset)
        return;
    
    if(race_started == 0){
        int fd = open(PIPE_NAME, O_WRONLY);
        //!!!!!!!!!!!!!!!!! TESTAR FD !!!!!!!!!!!!!!!!!!!
        write_pipe(fd, "END RACE");
    }
    pthread_cond_broadcast(&shared_memory->new_command);
    write_log("RACE SIMULATOR: SIGINT\n");

    while(wait(NULL) != -1);
    
    clean();

    write_debug("RACE SIMULATOR LEFT\n");

    exit(0);
}

void signal_tstp() {
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    pthread_mutex_lock(&shared_memory->mutex);
    if(shared_memory->race_started) {
        pthread_mutex_unlock(&shared_memory->mutex);
        show_statistics(shared_memory, config);
    } else {
        pthread_mutex_unlock(&shared_memory->mutex);
        write_log("RACE NOT STARTED\n");
    }
    signal(SIGINT, signal_sigint);
    signal(SIGTSTP, signal_tstp);
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

    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);

    config = load_config();
    if(config == NULL) return -1;

    init();

    malfunction_manager_pid = fork();
    if(malfunction_manager_pid == 0) {
        malfunction_manager(shared_memory, config);
        exit(0);
    }

    clock_pid = fork();
    if(clock_pid == 0) {
        sync_clock(shared_memory, config);
        exit(0);
    }
    
    race_manager_pid = fork();
    if(race_manager_pid == 0) {
        race_manager(shared_memory, config, malfunction_manager_pid);
        exit(0);
    }

    signal(SIGTSTP, signal_tstp);
    signal(SIGINT, signal_sigint);

    waitpid(race_manager_pid, NULL, 0);
    write_debug("RACE MANAGER IS LEAVING [%d]\n", race_manager_pid);

    waitpid(malfunction_manager_pid, NULL, 0);
    write_debug("MALFUNCTION MANAGER IS LEAVING [%d]\n", malfunction_manager_pid);

    waitpid(clock_pid, NULL, 0);
    write_debug("CLOCK IS LEAVING [%d]\n", clock_pid);

    clean();

    return 0;
}