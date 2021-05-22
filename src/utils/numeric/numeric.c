/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "numeric.h"

/**
 * @brief Check if the given string is a number
 * 
 * @param string String to check
 * @return int Logical value of the comparsion
 * 0 If the string is not a number
 * 1 If the string is a number
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

/**
 * @brief Check if the given string is a float
 * 
 * @param string String to check
 * @return int Logical value of the comparsion
 * 0 If the string is not a float
 * 1 If the string is a float
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