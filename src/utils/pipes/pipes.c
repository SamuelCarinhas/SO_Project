/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/
#include "pipes.h"

/**
 * @brief Reads information from the named pipe and unnamed pipes and process
 * that information using the given functions for each pipe
 * 
 * @param shared_memory Pointer to the shared memory structure
 * @param pipes Pointer to the pipes file descriptors [Namedpipe should be in the position 0]
 * @param n_pipes Number of pipes
 * @param handle_unnamed_pipe Function to handle the information read from the unnamed pipes
 * @param handle_named_pipe Function to handle the information read from the named pipes
 */
void read_from_pipes(shared_memory_t * shared_memory, int * pipes, int n_pipes, int (* handle_unnamed_pipe)(char * str), int (* handle_named_pipe)(char * str)) {
    char string[MAX_STRING];
    int n_read, res = OK;
    fd_set read_set;
    while(1) {
        FD_ZERO(&read_set);
        for(int i = 0; i < n_pipes; i++)
            FD_SET(pipes[i], &read_set);

        if(select(pipes[n_pipes-1] + 1, &read_set, NULL, NULL, NULL) > 0) {

            if(shared_memory->race_started == 0 && shared_memory->end_race == 1) {
                return;
            }

            for(int i = 0; i < n_pipes; i++) {
                if(FD_ISSET(pipes[i], &read_set)) {
                    n_read = read(pipes[i], string, MAX_STRING);

                    if(n_read <= 0)
                        break;

                    string[n_read] = '\0';
                    remove_endline(string);
                    if(n_read > 0 && i == 0)
                        res = handle_named_pipe(string) || res;
                    else if(n_read > 0)
                        res = handle_unnamed_pipe(string) || res;


                    if(i == 0) {
                        close(pipes[i]);
                        pipes[i] = open(PIPE_NAME, O_RDONLY | O_NONBLOCK);
                    }
                }
            }

            if(res == END)
                return;
        }
    }
}

/**
 * @brief Parse the string like printf usage and write the information
 * to the given pipe
 * 
 * @param fd Pipe file descriptor
 * @param format String format
 * @param ... String arguments
 */
void write_pipe(int fd, char * format, ...) {
    char buffer[MAX_STRING];
    va_list arg;
    va_start(arg, format);
    vsnprintf(buffer, MAX_STRING, format, arg);
    va_end(arg);
    write(fd, buffer, MAX_STRING);
}