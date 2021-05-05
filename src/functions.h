/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#ifndef FUNCTIONS_HEADER
#define FUNCTIONS_HEADER

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#include "global.h"

#define LOG_FILE "log/log.txt"

sem_t mutex_log;

int read_line(FILE *, char *, int);
void init_mutex_log();
void destroy_mutex_log();
void write_log(char *, ...);
char * trim (char * );
int starts_with(char * , char * );
int is_number(char *);
team_t * get_teams(shared_memory_t *);
car_t * get_cars(shared_memory_t *, config_t * );
car_t * get_car(shared_memory_t *, config_t *, int, int);

#endif