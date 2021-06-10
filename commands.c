#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#define MAX_STRING 5000

char pipe_name[] = "COMMAND_PIPE";

int read_line(FILE * file, char * line, int max_len) {
    int c, i = 0;

    while((c = fgetc(file)) != '\n' && c != '\r') {
        if(c == EOF)
            return c;
        if(i == max_len)
            break;
        line[i++] = (char) c;
    }

    if(i >= max_len - 1) {
        if(c != '\n')
            while(fgetc(file) != '\n');
        return -1;
    } else if(c == '\r')
        fgetc(file);

    line[i] = '\0';
    return 0;
}

int main(int argc, char * argv[]) {
    assert(argc == 2);

    FILE * file = fopen(argv[1], "r");
    
    assert(file != NULL);

    char line[MAX_STRING] = {0};

    int fd = open(pipe_name, O_WRONLY);

    while(read_line(file, line, MAX_STRING) == 0) {
        printf("%s\n", line);
        write(fd, line, MAX_STRING);
    }


    return 0;
}
