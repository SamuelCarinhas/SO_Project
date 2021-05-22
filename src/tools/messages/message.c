/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/
#include "message.h"

/**
 * @brief Sends messages using message queue
 * 
 * @param mq_id Message queue identifier
 * @param message Message to send
 */
void send_message(int mq_id, message_t * message) {
    msgsnd(mq_id, message, sizeof(message_t) - sizeof(long), 0);
}

/**
 * @brief Reads the messages of a certain car from the message queue
 * 
 * @param mq_id Message queue identifier
 * @param message Pointer to the message structure to store the message
 * @param car_number Car number
 * @return int positive integer if all goes well
 */
int receive_message(int mq_id, message_t * message, int car_number) {
    return msgrcv(mq_id, message, sizeof(message_t) - sizeof(long), car_number, IPC_NOWAIT);
}
