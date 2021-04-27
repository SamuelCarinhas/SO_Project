/*
    SO PROJECT 2021

    Students:
        Joana Maria Silva Simoes 2019217013
        Samuel dos Santos Carinhas 2019217199
*/

#include "malfunction_manager.h"

/*
* NAME :                            void race_manager(shared_memory_t * shared_memory)
*
* DESCRIPTION :                     Function to handle the Malfunction process
*
* PARAMETERS:
*           shared_memory_t *       shared_memory          pointer to the shared memory
*       
* RETURN :
*          void
*
* TODO :                            EVERYTHING
*
*/
void malfunction_manager(shared_memory_t * shared_memory, config_t * config) {
    #ifdef DEBUG
        write_log("DEBUG: Malfunction manager created [%d], Time units: %d\n", getpid(), config->malfunction_time_units);
    #endif
}
