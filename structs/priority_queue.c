#include <stdio.h>
#include <stdlib.h>


struct element {
    int x;
    int priority;
    struct element * next;
};

struct list {
    struct element * start;
};

void add_element ( struct list * wee, struct element * x ) {
    struct element * back = NULL;
    struct element * current = wee->start;

    if ( current == NULL ) {
        wee->start = x;
        return;
    }

    while ( current != NULL ) {
        if ( current->priority < x->priority ) {
            if ( back == NULL ) {
                wee->start = x;
                x->next = current;
            } else {
                back->next = x;
                x->next = current;
            }
            return;
        }
        back = current;
        current = current->next;    
    }
    
    back->next = x;
}

struct element * create_element ( int x, int p )  {
    struct element * e = malloc ( sizeof ( struct element )  ) ;
    e->x = x;
    e->priority = p;
    e->next = NULL;
    return e;
}

void print_list ( struct list * wee )  {
    struct element * x = wee->start;
    while ( x != NULL )  {
        printf ( "%d, %d\n", x->x, x->priority ) ;
        x = x->next;
    }
}

int main (  )  {
    struct list wee;
    wee.start = NULL;

    add_element ( &wee, create_element ( 10, 2 )  ) ;
    add_element ( &wee, create_element ( 13, 9 )  ) ;
    add_element ( &wee, create_element ( 7, 1 )  ) ;
    add_element ( &wee, create_element ( 9, 2 )  ) ;
    add_element ( &wee, create_element ( 2, 3 )  ) ;

    print_list ( &wee ) ;

    return 0;
}