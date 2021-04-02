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

void init_mutex_log() {
    sem_init(&mutex_log, 1, 1);
}

void destroy_mutex_log() {
    sem_destroy(&mutex_log);
}

void write_log(char * string) {
    time_t now = time(NULL);
    struct tm * tm_struct = localtime(&now);

    sem_wait(&mutex_log);
    FILE * log = fopen(LOG_FILE, "a");
    fprintf(log, "%2d:%2d:%2d %s\n", tm_struct->tm_hour, tm_struct->tm_min, tm_struct->tm_sec, string);
    printf("%2d:%2d:%2d %s\n", tm_struct->tm_hour, tm_struct->tm_min, tm_struct->tm_sec, string);
    fclose(log);
    sem_post(&mutex_log);
}

char * trim(char * string) {

    while(*string == ' ') string++;

    char * current;
    for(current = string + strlen(string) - 1; *current == ' ' && current >= string; current--);
    *(current+1) = '\0';

    return string;
}

int starts_with(char * a, char * b) {
    unsigned int len_b = strlen(b);
    if(len_b > strlen(a)) return 0;

    for(unsigned int i = 0; i < len_b; i++)
        if(a[i] != b[i]) return 0;

    return 1;
}
