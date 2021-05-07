/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/
#include "message.h"

void send_message(int mq_id, message_t * message) {
    msgsnd(mq_id, message, sizeof(message_t) - sizeof(long), 0);
}
