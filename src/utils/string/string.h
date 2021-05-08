/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#ifndef STRING_HEADER
#define STRING_HEADER

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

extern char * trim(char * string);
extern int starts_with(char * a, char * b);
extern int read_line(FILE * file, char * line, int max_len);
extern void remove_endline(char * string);
extern int ends_with(char * a, char * b);

#endif