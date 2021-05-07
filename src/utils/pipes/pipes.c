#include "pipes.h"

void read_from_pipes(int named, int * pipes, int n_pipes, int (* handle_read)(char * str)) {
    char string[MAX_STRING];
    while(1) {
        fd_set read_set;
        FD_ZERO(&read_set);
        for(int i = 0; i < n_pipes; i++)
            FD_SET(pipes[i], &read_set);
        if(select(pipes[n_pipes-1] + 1, &read_set, NULL, NULL, NULL) > 0){
            for(int i = 0; i < n_pipes; i++){
                if(FD_ISSET(pipes[i], &read_set)){
                    read(pipes[i], string, MAX_STRING);
                    if(handle_read(string)) {
                        return;
                    }
                }
                if(named) {
                    close(pipes[i]);
                    pipes[i] = open(PIPE_NAME, O_RDONLY|O_NONBLOCK);
                }
            }
        }
    }
}