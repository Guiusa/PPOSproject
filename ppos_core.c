// GRR20210572 Guiusepe Oneda Dal Pai

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include "ppos_data.h"
#include "ppos.h"
#include "queue.h"

int gbl_tid_next = 1;               // Controle de id de tasks criadas
task_t *out_task, main_task;        // Variáveis para task switching 
task_t dispatcher, *task_queue ;    // Variáveis pro dispatcher
struct sigaction action ;           // Tratador de sinais
struct itimerval timer ;            // Timer, simula timer do hardware

/*
 * É chamada a cada 1ms
 * decrementa quantum da tarefa atual e faz switch se necessário
 */
void tick_handler (int signum) {
    // Se a task não é de sistema
    if(out_task->sys_task) return ;
    // Se quantum ainda é válido
    if(--out_task->quantum > 0) return ;

    // Chegou em 0, faz task yield pra preempção
    task_yield() ;
}
//##############################################################################



/*
 * Função auxiliar para imprimir a fila caso necessário
 */
void print_elem (void *ptr){
    task_t *elem = ptr ;

    if(!elem) return;

    elem->prev ? printf ("%d", elem->prev->id) : printf("*");
    printf("<%d>", elem->id) ;
    elem->next ? printf("%d", elem->next->id) : printf("*");
}
//##############################################################################


                    
/*
 * Retorna endereço para inserir task na fila
 * algoritmo é inserir no fim por enquanto
 */
task_t* scheduler(){
    task_t *iter = task_queue;
    task_t *menor = task_queue;

    // Itera sobre a fila para achar a tarefa com mais prioridade | "menor"
    do {
        int total_iter = iter->prio_s + iter->prio_d ;
        int total_menor = menor->prio_s + menor->prio_d ;
        if(total_iter < total_menor) menor = iter ;
        iter = iter->next ;
    } while (iter != task_queue) ;
    
    // Itera para envelhecer as tarefas e resetar a menor
    iter = task_queue ;
    do {
        // Se for a tarefa escolhida, reseta prioridade dinâmica
        if(iter == menor){
            iter->prio_d = PRIO_DEFAULT ;
            iter = iter->next ;
            continue ;
        }

        if(iter->prio_d + iter->prio_s > PRIO_ALTA)
            iter->prio_d += PRIO_PASSO;
        
        iter = iter->next ;
    } while (iter != task_queue) ;
    
    return menor;
}
//##############################################################################



/*
 * Corpo do dispatcher, itera sobre a fila para colocar tarefas em execução
 */
void dispatcher_body(){
    // Remove dispatcher da fila
    queue_remove((queue_t **) &task_queue, (queue_t*) &dispatcher);
    task_t* task = NULL ;

    while(queue_size((queue_t *) task_queue) > 0){
        // Seleciona task pelo scheduler, faz o switch
        task = scheduler() ;
        queue_remove((queue_t **) &task_queue, (queue_t *) task);

        task_switch(task);        

        // Altera a task após sair de execução
        switch (task->status){
            case TASK_PRONTA:
                queue_append((queue_t **) &task_queue, (queue_t *) task) ;
                break ;

            case TASK_TERMINADA:
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
 
    // Inicializa tratador de sinais
    action.sa_handler = tick_handler ;
    sigemptyset (&action.sa_mask) ;
    action.sa_flags = 0 ;
    if(sigaction(SIGALRM, &action, 0) < 0){
        perror("ERRO em sigaction: ") ;
        exit(1) ;
    }

    // Inicializa o timer que simula ticks do hardware
    timer.it_value.tv_sec = FIRST_TICK_S ;
    timer.it_interval.tv_sec = INTERVAL_TICK_S ;
    timer.it_value.tv_usec = FIRST_TICK_US ;
    timer.it_interval.tv_usec = INTERVAL_TICK_US ;

    if(setitimer(ITIMER_REAL, &timer, 0) < 0){
        perror("ERRO em setitimer: ") ;
        exit(1) ;
    }

    // Inicia a task do dispatcher
    task_queue = NULL;
    task_init(&dispatcher, (void *) dispatcher_body, NULL);
    dispatcher.sys_task = 1 ; // dispatcher não pode sofrer preemção
    #ifdef DEBUG
        printf("[ppos_init]\tIniciando e definindo task dispatcher\n");
    #endif

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
    task->status = TASK_PRONTA ;
    task_setprio(task, PRIO_DEFAULT) ;
    task->quantum = 0 ;
    task->sys_task = 0 ;

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
    task->status = TASK_RODANDO ;
    task->quantum = QUANTUM ;

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
    out_task->status = TASK_PRONTA ;
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
            out_task->status = TASK_TERMINADA ;
            task_switch(&dispatcher) ;
            break;

        case 1: // id 1 task dispatcher
            exit(exit_code) ;
            break;  

        default: // outras tasks
            out_task->status = TASK_TERMINADA ;
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
    if(!task) task = out_task;

    task->prio_d = 0 ;
    // Testa limites {prio e [-20, 20]}
    if(prio < PRIO_ALTA){
        task->prio_s = PRIO_ALTA ;
    } else if (prio > PRIO_BAIXA) {
        task->prio_s = PRIO_BAIXA ;
    } else task->prio_s = prio ;
}
//##############################################################################



/*
 * Retorna o valor de prio(rity) da task recebida, se for nula retorna da task
 * corrente
 */
int task_getprio(task_t* task){
    return (task) ? task->prio_s : out_task->prio_s ;
}
//##############################################################################
