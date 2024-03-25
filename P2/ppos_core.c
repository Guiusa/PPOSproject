// GRR20210572 Guiusepe Oneda Dal Pai
#include <stdio.h>
#include <stdlib.h>
#include "ppos_data.h"
#include "ppos.h"

int gbl_tid_next = 1;
task_t *out_task, main_task;

void ppos_init(){
    // Desativa o buffer usado pelo printf
    setvbuf(stdout, 0, _IONBF, 0) ;

    // Inicializando a task main
    getcontext(&main_task.context);
    out_task = &main_task;
    main_task.id = 0;

    #ifdef DEBUG
        printf("[ppos_init]\tIniciando o sistema e a task main\n");
    #endif
}

int task_init(task_t *task, void (*start_func)(void *), void *arg){
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

    makecontext(&task->context, (void*)(*start_func), 1, arg) ;

    task->id = gbl_tid_next++;
    task->status = 0;

    #ifdef DEBUG
        printf("[task_init]\tTarefa %d criada\n", task->id);
    #endif

    return task->id;
}

int task_switch (task_t *task){
    #ifdef DEBUG
        printf("[task_switch]\tTrocando contexto %d -> %d\n", out_task->id, task->id);
    #endif
    task_t* aux = out_task;
    out_task = task;
    swapcontext(&aux->context, &task->context);
    
    return 0;
}

void task_exit(int exit_code){
    #ifdef DEBUG
        printf("[task_exit]\tTerminando tarefa %d\n", out_task->id);
    #endif
    task_switch(&main_task);
}

int task_id(){
    return out_task->id;
}
