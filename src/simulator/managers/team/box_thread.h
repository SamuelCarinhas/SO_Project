#ifndef BOX_THREAD_HEADER
#define BOX_THREAD_HEADER

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
#include "../../race_simulator.h"
#include "../../../utils/mutexes/mutex.h"
#include "../../../utils/log/log.h"
#include "../../../tools/messages/message.h"
#include "../../../tools/shared/memory.h"
#include "../../../config/config.h"
#include "../../../utils/pipes/pipes.h"
#include "../../../utils/string/string.h"
#include "../../../utils/clock/clock.h"

typedef struct box_args box_args_t;

struct box_args {
    shared_memory_t * shared_memory;
    config_t * config;
    team_t * team;
    int fd;
};

extern box_args_t * create_box_args(shared_memory_t * shared_memory, config_t * config, team_t * team, int fd);
extern void update_box();
extern int join_box(car_t * car);
extern void * box_handler(void * args);

#endif