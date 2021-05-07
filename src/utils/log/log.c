/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/
#include "log.h"

/*
* NAME :                            void init_mutex_log()
*
* DESCRIPTION :                     Function create the mutex for the log file
*       
* PARAMETERS :
*           void
*
* RETURN :
*           void
*
*/
void init_mutex_log() {
    sem_init(&mutex_log, 1, 1);
}

/*
* NAME :                            void destroy_mutex_log()
*
* DESCRIPTION :                     Function destroy the mutex of the log file
*       
* PARAMETERS :
*           void
*
* RETURN :
*           void
*
*/
void destroy_mutex_log() {
    sem_destroy(&mutex_log);
}

void private_write_log(struct tm * tm_struct, char * format, va_list arg, int debug) {
    FILE * log = fopen(LOG_FILE, "a");
    fprintf(log, "%02d:%02d:%02d ", tm_struct->tm_hour, tm_struct->tm_min, tm_struct->tm_sec);
    if(debug)
        fprintf(log, "DEBUG: ");
    vfprintf(log, format, arg);
    fclose(log);
}

void private_write_stdout(struct tm * tm_struct, char * format, va_list arg, int debug) {
    fprintf(stdout, "%02d:%02d:%02d ", tm_struct->tm_hour, tm_struct->tm_min, tm_struct->tm_sec);
    if(debug)
        fprintf(stdout, "DEBUG: ");
    vfprintf(stdout, format, arg);
}

/*
*
* NAME :                            int write_log(FILE * file, char * line, int max_len)
*
* DESCRIPTION :                     Function to write information in the log file using a mutex to
*                                   prevent simultaneous access
*
* PARAMETERS :
*           char *                  format                  pointer to file
*           ...                     Arguments for the string format
*       
* RETURN :
*           void
*
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