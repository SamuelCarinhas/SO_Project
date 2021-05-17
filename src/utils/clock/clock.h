/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/
#ifndef CLOCK_HEADER
#define CLOCK_HEADER

#include <stdio.h>
#include "../../tools/shared/memory.h"
#include "../../config/config.h"

void sync_clock(shared_memory_t * shared, config_t * config);
void sync_sleep(shared_memory_t * shared, int time);

#endif