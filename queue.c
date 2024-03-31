// GRR20210572 Guiusepe Oneda Dal Pai
#include <stdio.h>
#include "queue.h"

/*
 * Conta o número de elementos na fila
 */
int queue_size (queue_t *queue){
    queue_t *aux = queue;
    int s = 0;
    
    if(aux == NULL) return 0; // fila vazia

    do { 
        aux = aux->next;
        s++;
    } while(aux != queue);

    return s;
}
//##############################################################################



/*
 * Itera sobre a fila, chamando print_elem para cada nodo
 */
void queue_print(char *name, queue_t *queue, void print_elem (void*) ){
    printf("%s [", name);
    queue_t* aux = queue;
    
    if(! queue) goto queue_print_end; // Fila vazia ainda imprime "[]"

    do 
    {
        print_elem((void *) aux);
        printf(" ");
        aux = aux->next;
    } while(aux != queue);

    // \b é um backspace, gambiarra
    printf("\b \b");

    queue_print_end:
    printf("]\n"); 

}
//##############################################################################




/*
 * Adiciona nodo ao fim da fila
 */
int queue_append (queue_t **queue, queue_t *elem){
    // Checa se: fila existe | elem existe | elem não está enfileirado
    if(!queue) return -1;
    if(!elem) return -2;
    if(elem->prev || elem->next) return -3;

    // Se a fila está vazia, os componentes do nodo apontam para si
    if(! *queue)
    {
        *queue = elem;
        elem->next = elem;
        elem->prev = elem;
        return 0;
    }

    queue_t* aux = *queue;
    aux = aux->prev; // fim da fila

    elem->prev = aux;
    elem->next = aux->next;
    aux->next = elem;
    elem->next->prev = elem;
        
    return 0;
}
//##############################################################################



/*
 * Remove elemento da fila
 */
int queue_remove (queue_t **queue, queue_t *elem){
    // Checa se: fila existe | elem existe | fila não está vazia
    if(!queue) return -1;
    if(!elem) return -2;
    if(!*queue) return -3;
    
    queue_t* aux = *queue;
    int onqueue = 0;
    // Checa se o elemento recebido realmente está na fila recebida
    do {
        if(aux == elem){
            onqueue = 1;
            break;
        }
        aux = aux->next;
    } while (aux != *queue) ;

    if(!onqueue) return -4 ;

    // Se elem é o primeiro nodo da fila, é necessário reposicionar o ponteiro
    // Se elem é o único nodo, queue é setado para NULL
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
