/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/
#include "string.h"

/*
*
* NAME :                            char * trim(char * string)
*
* DESCRIPTION :                     Deletes whitespaces at the beginnig and at the end
*
* PARAMETERS :
*           char *                  string                  pointer to a string
*
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

void remove_endline(char * string) {

    char * current = string;
    while(*current) {
        if(*current == '\r' || *current == '\n') {
            *current = '\0';
            return;
        }
        current++;
    }

    return;
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
* NAME :                            int ends_with(char * a, char * b)
*
* DESCRIPTION :                     Checks if string 'a' ends with string 'b'
*
* PARAMETERS :
*           char *                  a                  First String
*           char *                  b                  Second String
*
*       
* RETURN :
*           int                     1 if 'a' ends with 'b', 0 otherwise
*
*/
int ends_with(char * a, char * b) {
    int len_b = (int) strlen(b);
    int len_a = (int) strlen(a);

    if(len_b > len_a)
        return 0;

    for(int i = len_b - 1, j = len_a -1; i>=0 ; i--, j--)
        if(a[j] != b[i])
            return 0;
    return 1;
}

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