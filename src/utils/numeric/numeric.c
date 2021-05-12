/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "numeric.h"

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
    for(char * p = string; *p != '\0'; p++) {
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