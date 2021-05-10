#include "pipes.h"

int read_from_pipes(int * pipes, int n_pipes, int (* handle_unnamed_pipe)(char * str), int (* handle_named_pipe)(char * str)) {
    char string[MAX_STRING];
    int n_read, res;
    fd_set read_set;
    while(1) {
        FD_ZERO(&read_set);
        for(int i = 0; i < n_pipes; i++)
            FD_SET(pipes[i], &read_set);
        if(select(pipes[n_pipes-1] + 1, &read_set, NULL, NULL, NULL) > 0) {
            for(int i = 0; i < n_pipes; i++) {
                if(FD_ISSET(pipes[i], &read_set)) {
                    n_read = read(pipes[i], string, MAX_STRING);
                    string[n_read] = '\0';
                    remove_endline(string);
                    if(n_read > 0 && i == 0) {
                        res = handle_named_pipe(string);
                        if(res == END || res == FINISH)
                            return res;
                    } else if(n_read > 0) {
                        res = handle_unnamed_pipe(string);
                        if(res == END || res == FINISH)
                            return res;
                    }
                }
            }
        }
    }
}

void write_pipe(int fd, char * format, ...) {
    char buffer[MAX_STRING];
    va_list arg;
    va_start(arg, format);
    vsnprintf(buffer, MAX_STRING, format, arg);
    va_end(arg);
    write(fd, buffer, MAX_STRING);
}