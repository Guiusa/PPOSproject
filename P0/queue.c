// Guiusepe Oneda Dal Pai - GRR20210572
#include <stdio.h>
#include "queue.h"

//##############################################################################
// Counts number of elements in queue
int queue_size (queue_t *queue){
    queue_t *aux = queue;
    int s = 0;
    
    if(aux == NULL) return 0; // empty queue

    do { 
        aux = aux->next;
        s++;
    } while(aux != queue);

    return s;
}
//##############################################################################



//##############################################################################
// Iters over queue, calling print_elem function received for each node
void queue_print(char *name, queue_t *queue, void print_elem (void*) ){
    printf("%s [", name);
    queue_t* aux = queue;
    
    if(! queue) goto queue_print_end; // Empty queue still prints "[]"

    do 
    {
        print_elem((void *) aux);
        printf(" ");
        aux = aux->next;
    } while(aux != queue);

    // \b is a backspace, removes leftover blank space in last node
    printf("\b \b");

    queue_print_end:
    printf("]\n"); 

}
//##############################################################################



//##############################################################################
// Appends a node to the end of the queue
int queue_append (queue_t **queue, queue_t *elem){
    // Checks if: queue exists | elem exists | elem isn't in another queue
    if(!queue) return -1;
    if(!elem) return -2;
    if(elem->prev || elem->next) return -3;

    // If queue is empty, node pointer refers to itself
    if(! *queue)
    {
        *queue = elem;
        elem->next = elem;
        elem->prev = elem;
        return 0;
    }

    queue_t* aux = *queue;
    aux = aux->prev; // End of queue

    elem->prev = aux;
    elem->next = aux->next;
    aux->next = elem;
    elem->next->prev = elem;
        
    return 0;
}

//##############################################################################
// Removes a queue's node
int queue_remove (queue_t **queue, queue_t *elem){
    // Checks if: queue exists | elem exists | queue isn't empty
    if(!queue) return -1;
    if(!elem) return -2;
    if(!*queue) return -3;
    
    queue_t* aux = *queue;
    int onqueue = 0;
    // Checks if elem is on given queue, throws exception if not
    do {
        if(aux == elem){
            onqueue = 1;
            break;
        }
        aux = aux->next;
    } while (aux != *queue) ;

    if(!onqueue) return -4 ;

    // If elem is the first node of the queue, needs to reposition queue pointer
    // If elem is the only node, queue is set to NULL
    if(*queue == elem){
       if(elem->next == elem){
            *queue = NULL;
       } else {
            *queue = elem->next;
       }
    }

    elem->prev->next = elem->next;
    elem->next->prev = elem->prev;
	elem->prev = NULL;
	elem->next = NULL;
    return 0;
}
//##############################################################################
