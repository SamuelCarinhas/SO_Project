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
#include "failure_manager.h"


int main() {

    config_t * config = load_config();
    if(config == NULL) return -1;

    pid_t race_manager_pid;

    race_manager_pid = fork();
    if(race_manager_pid == 0) {
        race_manager(config);
        exit(0);
    }

    pid_t failure_manager_pid;

    failure_manager_pid = fork();
    if(failure_manager_pid == 0) {
        failure_manager();
        exit(0);
    }

    int stat;
    waitpid(race_manager_pid, &stat, 0);
    if(WIFEXITED(stat))
        printf("Race manager %d terminated with status: %d\n", race_manager_pid, WEXITSTATUS(stat));
    waitpid(failure_manager_pid, &stat, 0);
    if(WIFEXITED(stat))
        printf("Failure manager %d terminated with status: %d\n", failure_manager_pid, WEXITSTATUS(stat));

    return 0;
}