#include "priority_queue.h"

void add_element(priority_queue_t * queue, priority_element_t * element) {
    priority_queue_t * back = NULL;
    priority_queue_t * current = queue->start;

    if (current == NULL) {
        queue->start = element;
        return;
    }

    while (current != NULL) {
        if (current->priority < element->priority) {
            if (back == NULL) {
                current->start = element;
                element->next = current;
            } else {
                back->next = element;
                element->next = current;
            }
            return;
        }
        back = current;
        current = current->next;    
    }
    
    back->next = element;
}

priority_queue_t * create_element(car_t * car, int priority) {
    pq_element_t * element = malloc (sizeof(pq_element_t)) ;
    element->car = car;
    element->priority = priority;
    element->next = NULL;
    return element;
}