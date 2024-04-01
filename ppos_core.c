// GRR20210572 Guiusepe Oneda Dal Pai
// Esse software foi produzido ao som dessa(s) bomba(s):
// https://open.spotify.com/playlist/37i9dQZF1DZ06evO0gHlRP?si=6940c39240224a56
// https://open.spotify.com/album/4d1mlXFZNIpwadYTMl8pED?si=0dIQdTG0R7CQRPwNfSkn-w

#include <stdio.h>
#include <stdlib.h>
#include "ppos_data.h"
#include "ppos.h"
#include "queue.h"

/* pronta rodando suspensa terminada */
#define TERMINADA   0
#define PRONTA      1
#define RODANDO     2
#define SUSPENSA    3

#define TASK_STACK_SIZE 64*1024

                                    
int gbl_tid_next = 1;               // Controle de id de tasks criadas
task_t *out_task, main_task;        // variáveis para task switching 
task_t dispatcher, *task_queue ;    // variáveis pro dispatcher

void print_elem (void *ptr){
    task_t *elem = ptr ;

    if(!elem) return;

    elem->prev ? printf ("%d", elem->prev->id) : printf("*");
    printf("<%d|%d>", elem->id, elem->prio) ;
    elem->next ? printf("%d", elem->next->id) : printf("*");
}
                    
/*
 * Retorna endereço para inserir task na fila
 * algoritmo é inserir no fim por enquanto
 */
task_t* scheduler(){
    return task_queue;
}
//##############################################################################

void ordered_append(task_t **queue, task_t *task){
    if(queue_size((queue_t *) *queue) == 0){
        queue_append((queue_t **) queue, (queue_t *) task);
        return;
    }
    task_t* iter = *queue ;
    while(iter->next != *queue){
        if(iter->prio <= task->prio) break ;
        iter = iter->next;
    }
    queue_append((queue_t **) &iter, (queue_t *) task);

    queue_print("FILA: ", (queue_t *) task_queue, print_elem);
}

/*
 * Corpo do dispatcher, itera sobre a fila para colocar tarefas em execução
 */
void dispatcher_body(){
    queue_remove((queue_t **) &task_queue, (queue_t*) &dispatcher);
    task_t* task = NULL ;

    while(queue_size((queue_t *) task_queue) > 0){
        task = scheduler() ;
        queue_remove((queue_t **) &task_queue, (queue_t *) task);

        task_switch(task);        

        switch (task->status){
            case PRONTA:
                ordered_append(&task_queue, task);
                //queue_append((queue_t **) &task_queue, (queue_t *) task) ;
                break ;

            case TERMINADA:
                free(task->context.uc_stack.ss_sp) ;
                task = NULL ;
                break ;
        }
    }

    task_exit(0);
}
//##############################################################################



/*
 * Faz inicializações do sistema ppos
 */
void ppos_init(){
    // Desativa buffer do printf
    setvbuf(stdout, 0, _IONBF, 0) ;

    #ifdef DEBUG
        printf("[ppos_init]\tIniciando o sistema e a task main\n");
    #endif

    // Inicia a task main
    getcontext(&main_task.context) ;
    out_task = &main_task;
    main_task.id = 0 ;

    #ifdef DEBUG
        printf("[ppos_init]\tIniciando e definindo task dispatcher\n");
    #endif

    // Inicia a task do dispatcher
    task_queue = NULL;
    task_init(&dispatcher, (void *) dispatcher_body, NULL);

    #ifdef DEBUG
        printf("[ppos_init]\tPPOS inicializado\n");
    #endif
}
//##############################################################################



/*
 * Inicializa uma nova task
 */
int task_init(task_t *task, void (*start_func)(void *), void *arg){
    // Inicializa o contexto e aloca uma pilha pra ele
    getcontext(&task->context) ;

    char *stack = malloc(TASK_STACK_SIZE) ;
    if(stack){
        task->context.uc_stack.ss_sp = stack ;
        task->context.uc_stack.ss_size = TASK_STACK_SIZE ;
        task->context.uc_stack.ss_flags = 0 ;
        task->context.uc_link = 0;
    } else {
        perror("Erro na criação da pilha para essa tarefa\n") ;
        exit(-1);
    }

    // Linka a função recebida com os argumentos adequados
    makecontext(&task->context, (void (*)(void)) start_func, 1, arg) ;

    // Inicialização dos componentes da struct task_t, adição na fila de prontas
    task->id = gbl_tid_next++ ;
    task->status = PRONTA ;
    task->prio = 0 ;

    ordered_append(&task_queue, task);
//    queue_append((queue_t **) &task_queue, (queue_t *) task) ;

    #ifdef DEBUG
        printf("[task_init]\tTarefa %d criada\n", task->id);
        printf("[task_init]\tFila de prontas tem tamanho %d\n", queue_size((queue_t *) task_queue)) ;
    #endif

    return task->id;
}
//##############################################################################



/*
 * Troca a task atual para a recebida por parâmetro
 */
int task_switch (task_t *task){
    task_t* aux = out_task ;
    out_task = task ;
    task->status = RODANDO ;

    #ifdef DEBUG
        printf("[task_switch]\tTrocando contexto %d -> %d\n", aux->id, task->id);
    #endif

    swapcontext(&aux->context, &task->context);
    
    return 0;
}
//##############################################################################



/*
 * troca task atual para dispatcher
 */
void task_yield(){
    out_task->status = PRONTA ;
    task_switch(&dispatcher);
}
//##############################################################################v




/*
 * Finaliza uma task, mudando para o dispatcher quando necessário
 */
void task_exit(int exit_code){
    #ifdef DEBUG
        printf("[task_exit]\tTerminando tarefa %d\n", out_task->id);
    #endif

    switch(task_id()){
        case 0: // id 0 task main
            out_task->status = TERMINADA ;
            task_switch(&dispatcher) ;
            break;

        case 1: // id 1 task dispatcher
            exit(exit_code) ;
            break;  

        default: // outras tasks
            out_task->status = TERMINADA ;
            task_switch(&dispatcher) ;
            break;
    }
}
//##############################################################################



/*
 * Retorna id da task atual
 */ 
int task_id(){
    return out_task->id;
}
//##############################################################################



/*
 * Define um valor para o campo prio(rity) da task recebida
 */
void task_setprio(task_t* task, int prio){
    if(!task){
        perror("[task_setprio]\tTarefa recebida não inicializada\n");
        exit(1);
    }

    task->prio = prio;
}
//##############################################################################



/*
 * Retorna o valor de prio(rity) da task recebida, se for nula retorna da task
 * corrente
 */
int task_getprio(task_t* task){
    return (task) ? task->prio : out_task->prio ;
}
//##############################################################################
