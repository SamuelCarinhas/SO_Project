/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "functions.h"

/*
* NAME :                            int read_line(FILE * file, char * line, int max_len)
*
* DESCRIPTION :                     Function to read a full line (until \n) from a file
*
* PARAMETERS :
*           FILE *                  file                    pointer to file
*           char *                  line                    pointer to the char array to store the line
*           int *                   max_len                 max size of the char array
*       
* RETURN :
*           int                     If there were no errors 0, otherwise -1
*
*/
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

/*
* Addapted from http://www.firmcodes.com/write-printf-function-c/
*
* NAME :                            int read_line(FILE * file, char * line, int max_len)
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

    va_list arg;
    va_start(arg, format);

    sem_wait(&mutex_log);
    
    FILE * log = fopen(LOG_FILE, "a");
    
    fprintf(log, "%02d:%02d:%02d ", tm_struct->tm_hour, tm_struct->tm_min, tm_struct->tm_sec);
    fprintf(stdout, "%02d:%02d:%02d ", tm_struct->tm_hour, tm_struct->tm_min, tm_struct->tm_sec);
    
    vfprintf(log, format, arg);
    vfprintf(stdout, format, arg);

    fclose(log);
    sem_post(&mutex_log);

    va_end(arg);
}

/*
*
* NAME :                            char * trim(char * string)
*
* DESCRIPTION :                     Deletes whitespaces atthe beginnig and at the end
*
* PARAMETERS :
*           char *                  string                  pointer to a string

*       
* RETURN :
*           char *                  pointer to string without whitespaces
*
*/
char * trim(char * string) {

    while(*string == ' ')
        string++;

    char * current;
    for(current = string + strlen(string) - 1; *current == ' ' && current >= string; current--);
    *(current+1) = '\0';

    return string;
}

/*
* NAME :                            int starts_with(char * a, char * b)
*
* DESCRIPTION :                     Checks if string 'a' starts with string 'b'
*
* PARAMETERS :
*           char *                  a                  First String
*           char *                  b                  Second String
*
*       
* RETURN :
*           int                     1 if 'a' starts with 'b', 0 otherwise
*
*/
int starts_with(char * a, char * b) {
    unsigned int len_b = strlen(b);
    if(len_b > strlen(a))
        return 0;

    for(unsigned int i = 0; i < len_b; i++)
        if(a[i] != b[i])
            return 0;

    return 1;
}

/*
* NAME :                            int is_number(char * string)
*
* DESCRIPTION :                     Checks if string is a number
*
* PARAMETERS :
*           char *                  string                  String to check
*
*       
* RETURN :
*           int                     1 if is a number, 0 otherwise
*
*/
int is_number(char * string) {
    if(string == NULL)
        return 0;
    string = trim(string);
    for(char * p = string; *p != '\0'; p++){
        if(!isdigit(*p))
            return 0;
    }
    return 1;
}

/*
* NAME :                            int is_float(char * string)
*
* DESCRIPTION :                     Checks if string is a float
*
* PARAMETERS :
*           char *                  string                  String to check
*
*       
* RETURN :
*           int                     1 if is a float, 0 otherwise
*
*/  
int is_float(char * string) {
    const char delim[2] = ".";
    char  * first, * second;
    first = strtok(string, delim);
    second = strtok(NULL, delim);

    if(first == NULL)
        return 0;
    if(second == NULL) 
        return is_number(first);
    return is_number(first) && is_number(second);
}

team_t * get_teams(shared_memory_t shared_memory) {
    return (team_t *) (shared_memory + 1);
}

car_t * get_cars(shared_memory_t * shared_memory, config_t * config) {
    return (car_t *) get_team(shared_memory) + config->teams;
}

car_t * get_car(team_t * team, shared_memory_t shared_memory, config_t * config, int pos) {
    return (car_t *) get_cars(shared_memory, config) + team->pos_array * config->max_cars_per_team + pos;
}