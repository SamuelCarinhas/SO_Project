/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "functions.h"

int read_line(FILE * file, char * line, int max_len) {
    int c, i = 0;

    while((c = fgetc(file)) != '\n' && c != '\r') {
        if(c == EOF) return c;
        if(i == max_len) break;
        line[i++] = (char) c;
    }

    if(i >= max_len - 1) {
        if(c != '\n')
            while(fgetc(file) != '\n');
        return -1;
    } else if(c == '\r') fgetc(file);


    line[i] = '\0';
    return 0;
}