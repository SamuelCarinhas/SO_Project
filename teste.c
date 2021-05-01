#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_STRING 100

int main() {

    if(fork() == 0) {
        printf("Weeee\n");
        sleep(1);
        exit(0);
    }

    if(fork() == 0) {
        printf("=D\n");
        sleep(5);
        exit(0);
    }

    printf("Waiting for children...\n");
    while(wait(NULL) != -1);
    printf("Done.\n");

    return 0;
}