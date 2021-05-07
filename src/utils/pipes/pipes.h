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
#include "../../utils/mutexes/mutex.h"
#include "../../utils/log/log.h"
#include "../../tools/shared/memory.h"
#include "../../config/config.h"
#include "../../utils/string/string.h"

void read_from_pipes(int * pipes, int n_pipes, int (* handle_read)(char * str));

#endif