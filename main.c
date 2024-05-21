#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"
#include "queue.h"
#define TRUE 1

typedef struct fila_int {
    struct fila_int *prev, *next ;
    int v ;
} fila_int ;

task_t p1, p2, p3, c1, c2 ;
semaphore_t s_vaga, s_buffer, s_item ;
fila_int *BUFFER = NULL;

void add_value(int i){
    fila_int k;
    k.v = i ;
    queue_append((queue_t **) &BUFFER, (queue_t *) &k) ;
}

void remove_value(int* i){
    (*i) = BUFFER->v ;
    queue_remove((queue_t **) &BUFFER, (queue_t *) BUFFER) ;
}

void body_produtor (void *arg){
    int item ;
    while(TRUE){
        task_sleep(1000) ;
        item = random()%100 ;

        sem_down(&s_vaga) ; // ocupa uma vaga
        
        sem_down(&s_buffer) ;
        add_value(item) ;
        sem_up(&s_buffer) ;

        sem_up (&s_item) ; // adicionou item
                    
        printf("%s produziou %d\n", (char *) arg, item) ;
    }
}

void body_consumidor (void *arg){
    int item ;
    while(TRUE){
        sem_down(&s_item) ;

        sem_down(&s_buffer) ;
        remove_value(&item) ;
        sem_up(&s_buffer) ;

        sem_up (&s_vaga) ;

        printf("%s consumiu %d\n", (char *) arg, item) ;
        task_sleep(1000) ;
    }
}

int main(int argc, char** argv){
    ppos_init() ;

    sem_init(&s_vaga, 5) ;
    sem_init(&s_buffer, 1) ;
    sem_init(&s_item, 0) ;

    task_init(&p1, body_produtor, "p1");
    task_init(&p2, body_produtor, "p2");
    task_init(&p3, body_produtor, "p3");
//    task_init(&c1, body_consumidor, "c1");
//    task_init(&c2, body_consumidor, "c2");    

    task_exit(0) ;
}
