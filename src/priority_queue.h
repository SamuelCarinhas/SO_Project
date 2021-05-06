#ifdef PRIORITY_QUEUE_HEADER
#define PRIORITY_QUEUE_HEADER

typedef struct pq_element_t {
    car_t * car;
    int priority;
    struct pq_element_t * next;
} pq_element_t;

typedef struct {
    pq_element_t * start;
} priority_queue_t;

void pq_add_element (priority_queue_t * queue, pq_element_t * element);
priority_queue_t * create_element(car_t * car, int priority);

#endif