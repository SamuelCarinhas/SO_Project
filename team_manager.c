/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "team_manager.h"

enum box_estado {
    OPEN, RESERVED, OCCUPIED
};

void team_manager(config_t * config, team_t * team) {
    printf("Team : %ld\nName: %s\n", (long) getpid(), team->name);
}