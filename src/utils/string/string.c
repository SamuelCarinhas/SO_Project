/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/
#include "string.h"

/**
 * @brief Delete whitespaces from the beginning and the end of the given string
 * 
 * @param string String to be trimmed
 * @return char* Trimed string
 */
char * trim(char * string) {

    while(*string == ' ')
        string++;

    char * current;
    for(current = string + strlen(string) - 1; *current == ' ' && current >= string; current--);
    *(current+1) = '\0';

    return string;
}

/**
 * @brief Remove the endline character from the given string
 * 
 * @param string String to remove the endline character
 */
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

/**
 * @brief Check if the given string 'a' starts with the given string 'b'
 * 
 * @param a First string
 * @param b Second string
 * @return int Logical value of the comparsion
 * 0 if its false, 1 if its true
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

/**
 * @brief Check if the given string 'a' ends with the given string 'b'
 * 
 * @param a First string
 * @param b Second String
 * @return int Logical value of the comparsion
 * 0 if its false, 1 if its true
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

/**
 * @brief Read full line until endoffile character from a given file
 * 
 * @param file Pointer to the file
 * @param line Pointer to char array to stor the line read
 * @param max_len Length of the char array
 * @return int Logical value of the result of reading the line
 * 0 If there is no errors
 * 1 If something went wrong
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