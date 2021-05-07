/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#ifdef MESSAGE_HEADER
#define MESSAGE_HEADER

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
#include "../../utils/pipes/pipes.h"
#include "../../utils/string/string.h"

typedef struct message message_t;

message_t {
    long car_number;
    int malfunction;
};

extern void send_message(int mq_id, message_t * message);

#endif