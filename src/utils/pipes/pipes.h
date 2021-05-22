#ifndef PIPES_HEADER
#define PIPES_HEADER

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
#include "../../config/config.h"
#include "../../tools/shared/memory.h"

#define END 1
#define OK 0

extern int read_from_pipes(shared_memory_t * shared_memory, int * pipes, int n_pipes, int (* handle_unnamed_pipe)(char * str), int (* handle_named_pipe)(char * str));
extern void write_pipe(int fd, char * format, ...);

#endif