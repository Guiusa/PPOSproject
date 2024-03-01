#include <stdio.h>
#include "queue.h"

int queue_size (queue_t *queue){
    queue_t *aux = queue;
    int s = 0;
    if(aux == NULL)
        return s;

    do { 
        aux = aux->next;
        s++;
    } while(aux != queue);


    return s;
}

void queue_print(char *name, queue_t *queue, void print_elem (void*) ){
    printf("%s:\n", name);
    queue_t* aux = queue;
    
    if(! queue) return;

    do 
    {
        print_elem((void *) aux);
        aux = aux->next;
    } while(aux != queue);
    printf("\n");

}

int queue_append (queue_t **queue, queue_t *elem){
    if(!queue) return -1;
    if(!elem) return -2;
    if(elem->prev || elem->next) return -3;

    if(! *queue)
    {
        *queue = elem;
        elem->next = elem;
        elem->prev = elem;
        return 0;
    }

    queue_t* aux = *queue;
    aux->prev = elem;

    while(aux->next != *queue)
        aux = aux->next;

    elem->next = aux->next;
    elem->prev = aux;
    aux->next = elem;
        
    return 0;
}

int queue_remove (queue_t **queue, queue_t *elem){
    if(!queue) return -1;
    if(!elem) return -2;
    
    elem->prev->next = elem->next;
    elem->next->prev = elem->prev;
    return 0;
}
