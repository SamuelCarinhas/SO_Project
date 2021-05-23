/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/
#include "log.h"

static void private_write_log(struct tm * tm_struct, char * format, va_list arg, int debug);
static void private_write_stdout(struct tm * tm_struct, char * format, va_list arg, int debug);
sem_t mutex_log;
int fd_log;

/**
 * @brief Initializes the log mutex
 * Opens log file descriptor
 * 
 */
void init_log() {
    fd_log = open(LOG_FILE, O_CREAT | O_WRONLY | O_APPEND, 0644);
    assert(fd_log >= 0);
    sem_init(&mutex_log, 1, 1);
}

/**
 * @brief Destroys the log mutex
 * Closes the log file descriptor
 * 
 */
void close_log() {
    sem_destroy(&mutex_log);
    close(fd_log);
}

/**
 * @brief Writes the given information to the log file
 * 
 * @param tm_struct Struct with the current time (hours, minutes and seconds)
 * @param format String format to write to the log file
 * @param arg Arguments of the format
 * @param debug Flag if its a debug message or not
 */
static void private_write_log(struct tm * tm_struct, char * format, va_list arg, int debug) {
    char message[MAX_STRING*2];
    snprintf(message, MAX_STRING*2, "%02d:%02d:%02d ", tm_struct->tm_hour, tm_struct->tm_min, tm_struct->tm_sec);

    if(debug)
        strcat(message, "DEBUG: ");

    char parsed_message[MAX_STRING];
    vsnprintf(parsed_message, MAX_STRING, format, arg);
    strcat(message, parsed_message);
    write(fd_log, message, strlen(message));
}

/**
 * @brief Writes the given information to the stdout
 * 
 * @param tm_struct Struct with the current time (hours, minutes and seconds)
 * @param format String format to write to the stdout
 * @param arg Arguments of the format
 * @param debug Flag if its a debug message or not
 */
static void private_write_stdout(struct tm * tm_struct, char * format, va_list arg, int debug) {
    fprintf(stdout, "%02d:%02d:%02d ", tm_struct->tm_hour, tm_struct->tm_min, tm_struct->tm_sec);
    if(debug)
        fprintf(stdout, "DEBUG: ");
    vfprintf(stdout, format, arg);
}

/**
 * @brief Gets the current time and print the given string to
 * the log file and stdout without the debug flag
 * 
 * @param format String format like printf
 * @param ... String arguments
 */
void write_log(char * format, ...) {
    time_t now = time(NULL);
    struct tm * tm_struct = localtime(&now);

    sem_wait(&mutex_log);
    
    va_list arg;

    va_start(arg, format);
    private_write_log(tm_struct, format, arg, 0);
    va_end(arg);
    va_start(arg, format);
    private_write_stdout(tm_struct, format, arg, 0);
    va_end(arg);
    
    sem_post(&mutex_log);
}

/**
 * @brief Gets the current time and print the given string to
 * the log file and stdout with the debug flag
 * 
 * @param format String format like printf
 * @param ... String arguments
 */
void write_debug(char * format, ...) {
    #ifdef DEBUG
        time_t now = time(NULL);
        struct tm * tm_struct = localtime(&now);

        sem_wait(&mutex_log);
        
        va_list arg;
        va_start(arg, format);
        private_write_log(tm_struct, format, arg, 1);
        va_end(arg);
        va_start(arg, format);
        private_write_stdout(tm_struct, format, arg, 1);
        va_end(arg);
        
        sem_post(&mutex_log);
    #endif
}