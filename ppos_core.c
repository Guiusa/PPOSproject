// GRR20210572 Guiusepe Oneda Dal Pai
// Esse software foi produzido ao som dessa bomba:
// https://open.spotify.com/playlist/37i9dQZF1DZ06evO0gHlRP?si=6940c39240224a56
#include <stdio.h>
#include <stdlib.h>
#include "ppos_data.h"
#include "ppos.h"

#define TASK_STOPPED    0
#define TASK_READY      1
#define TASK_RUNNING    2

int gbl_tid_next = 1;           // Controle de id de tasks criadas
task_t *out_task, main_task;    // variáveis para task switching

/*
 * Faz inicializações do sistema ppos
 */
void ppos_init(){
    // Desativa buffer do printf 
    setvbuf(stdout, 0, _IONBF, 0) ;

    // Inicia a task main
    getcontext(&main_task.context) ;
    out_task = &main_task;
    main_task.id = 0 ;
    main_task.status = TASK_RUNNING ;

    #ifdef DEBUG
        printf("[ppos_init]\tIniciando o sistema e a task main\n");
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
    makecontext(&task->context, (void*)(*start_func), 1, arg) ;

    // Inicialização dos componentes da struct task_t
    task->id = gbl_tid_next++ ;
    task->status = TASK_STOPPED ;

    #ifdef DEBUG
        printf("[task_init]\tTarefa %d criada\n", task->id);
    #endif

    return task->id;
}
//##############################################################################



/*
 * Troca a task atual para a recebida por parâmetro
 */
int task_switch (task_t *task){
    #ifdef DEBUG
        printf("[task_switch]\tTrocando contexto %d -> %d\n", out_task->id, task->id);
    #endif

    task_t* aux = out_task ;
    out_task = task ;
    aux->status = TASK_STOPPED ;
    out_task->status = TASK_RUNNING ;
    swapcontext(&aux->context, &task->context);
    
    return 0;
}
//##############################################################################



/*
 * Finaliza uma task
 */
void task_exit(int exit_code){
    #ifdef DEBUG
        printf("[task_exit]\tTerminando tarefa %d\n", out_task->id);
    #endif
    task_switch(&main_task);
}
//##############################################################################



/*
 * Retorna id da tarefa atual
 */
int task_id(){
    return out_task->id;
}
//##############################################################################
