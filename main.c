#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"
#include "queue.h"
#define TRUE 1

task_t p1, p2, p3, c1, c2 ;
semaphore_t s_vaga, s_buffer, s_item ;
int BUFFER[5] ;
int b_top = 0 ;

void add_value(int i){
    BUFFER[b_top] = i ;
    b_top++ ;
}

void remove_value(int* i){
    (*i) = BUFFER[0] ;
    for(int i = 0; i<b_top; i++) BUFFER[i] = BUFFER[i+1] ;
    b_top-- ;
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
                    
        printf("%s produziu %d\n", (char *) arg, item) ;
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
    task_init(&c1, body_consumidor, "\t\t\t\t\tc1");
    task_init(&c2, body_consumidor, "\t\t\t\t\tc2");    

    task_exit(0) ;
}
