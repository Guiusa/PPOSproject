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
                    
/*
 * Retorna endereço para inserir task na fila
 * algoritmo é inserir no fim por enquanto
 */
task_t* scheduler(){
    return task_queue;
}

void dispatcher_body(){
    queue_remove((queue_t **) &task_queue, (queue_t*) &dispatcher);
    task_t* task = NULL ;

    while(queue_size((queue_t *) task_queue) > 0){
        task = scheduler() ;
        queue_remove((queue_t **) &task_queue, (queue_t *) task);

        task_switch(task);        

        switch (task->status){
            case PRONTA:
                queue_append((queue_t **) &task_queue, (queue_t *) task) ;
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

    queue_append((queue_t **) &task_queue, (queue_t *) task) ;

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
        case 0:
            out_task->status = TERMINADA ;
            task_switch(&dispatcher) ;
            break;

        case 1:
            exit(exit_code) ;
            break;  

        default:
            out_task->status = TERMINADA ;
            task_switch(&dispatcher) ;
            break;
    }
}

int task_id(){
    return out_task->id;
}
//##############################################################################v
