// GRR20210572 Guiusepe Oneda Dal Pai

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include "ppos_data.h"
#include "ppos.h"
#include "queue.h"

int gbl_tid_next = 1;               // Controle de id de tasks criadas
task_t *out_task, main_task;        // Variáveis para task switching 
task_t dispatcher, *task_queue ;    // Variáveis pro dispatcher
struct sigaction action ;           // Tratador de sinais
struct itimerval timer ;            // Timer, simula timer do hardware
unsigned int clock = 0 ;            // Clock do sistema
unsigned int last_task_time = 0 ;   // Ultimo clock que uma tarefa entrou 
task_t* sleep_queue ;               // Fila de tarefas dormindo
int user_tasks  = 0 ;               // quantia de tasks ativas

/*
 * É chamada a cada 1ms
 * decrementa quantum da tarefa atual e faz switch se necessário
 */
void tick_handler (int signum) {
    clock++ ;

    // Se a task não é de sistema
    if(out_task->sys_task) return ;
    // Se quantum ainda é válido
    if(--out_task->quantum > 0) return ;

    #ifdef DEBUG
        printf("task %d foi decrementada %d vezes\n", out_task->id, QUANTUM - out_task->quantum) ;
    #endif

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

    while(user_tasks > 0){
        // Itera sobre a fila de adormecidas e acorda as necessárias
        // Só a cada 100 ms
        task_t* sleep_aux = sleep_queue ;
        for(int i = queue_size((queue_t *) sleep_queue); i>0; i--){
            // if necessario pois após task_awake a variavel sleep_task
            // perde referência da fila de adormecidas
            if(systime() >= sleep_aux->sleep_untill){
                #ifdef DEBUG
                    printf("[dispatcher]\tTask %d será acordada\n", sleep_aux->id) ;
                #endif
                sleep_aux = sleep_aux->next ;
                task_awake(sleep_aux->prev, (task_t **) &sleep_queue) ;
                continue ;
            }
            sleep_aux = sleep_aux->next ;
        }
        
        if(queue_size((queue_t *) task_queue) == 0) continue ;
        // Seleciona task pelo scheduler, faz o switch
        task = scheduler() ;
        #ifdef DEBUG
            printf("[dispatcher]\tscheduler retornou a task %d\n", task->id) ;
        #endif
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
        printf("[ppos_init]\tIniciando o sistema\n");
    #endif

    // Inicia a task main
    main_task.first_clock = systime() ;
    getcontext(&main_task.context) ;
    main_task.id = 0 ;
    main_task.status = TASK_PRONTA ;
    main_task.quantum = QUANTUM ;
    main_task.sys_task = 0 ;
    task_setprio(&main_task, PRIO_DEFAULT) ;
    main_task.ativ = 1 ;
    main_task.cpu_time = 0 ;
    main_task.first_clock = systime() ;
    main_task.suspended_queue = NULL ;
    queue_append((queue_t**) &task_queue, (queue_t *) &main_task) ;
    out_task = &main_task;
    #ifdef DEBUG
        printf("[ppos_init]\tTask main inicializada\n") ;
    #endif

    // Inicializa tratador de sinais
    action.sa_handler = tick_handler ;
    sigemptyset (&action.sa_mask) ;
    action.sa_flags = 0 ;
    if(sigaction(SIGALRM, &action, 0) < 0){
        perror("ERRO em sigaction: ") ;
        exit(1) ;
    }
    #ifdef DEBUG
        printf("[ppos_init]\tInicializado o tratador de sinais\n") ;
    #endif

    // Inicializa o timer que simula ticks do hardware
    timer.it_value.tv_sec = FIRST_TICK_S ;
    timer.it_interval.tv_sec = INTERVAL_TICK_S ;
    timer.it_value.tv_usec = FIRST_TICK_US ;
    timer.it_interval.tv_usec = INTERVAL_TICK_US ;
    if(setitimer(ITIMER_REAL, &timer, 0) < 0){
        perror("ERRO em setitimer: ") ;
        exit(1) ;
    }
    #ifdef DEBUG
        printf("[ppos_init]\tInicializado o timer periódico\n") ;
    #endif

    // Inicia a task do dispatcher
    task_init(&dispatcher, (void *) dispatcher_body, NULL);
    dispatcher.sys_task = 1 ; // dispatcher não pode sofrer preemção
    #ifdef DEBUG
        printf("[ppos_init]\tIniciando e definindo task dispatcher\n");
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
    task->first_clock = systime() ;
    task->id = gbl_tid_next++ ;
    task->status = TASK_PRONTA ;
    task_setprio(task, PRIO_DEFAULT) ;
    task->quantum = 0 ;
    task->sys_task = 0 ;
    task->ativ = 0 ;
    task->first_clock = systime() ;
    task->cpu_time = 0 ;
    task->suspended_queue = NULL ;


    queue_append((queue_t **) &task_queue, (queue_t *) task) ;

    user_tasks++ ;
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
    unsigned int aux_time = last_task_time ;
    task_t* aux = out_task ;
    out_task = task ;
    task->status = TASK_RODANDO ;
    task->quantum = QUANTUM ;
    task->ativ++ ;

    last_task_time = systime() ;
    aux->cpu_time += last_task_time - aux_time ;
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

    printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n",
        out_task->id,
        systime() - out_task->first_clock,
        out_task->cpu_time,
        out_task->ativ
    ) ;

    out_task->exit_code = exit_code ;
    switch(task_id()){
        case 0: // id 0 task main
            out_task->status = TASK_TERMINADA ;
            queue_remove((queue_t **) &task_queue, (queue_t *) out_task) ;
            user_tasks--;
            task_switch(&dispatcher) ;
            break;

        case 1: // id 1 task dispatcher
            exit(exit_code) ;
            break;  

        default: // outras tasks
            while(out_task->suspended_queue){
                #ifdef DEBUG
                    printf("[task_exit]\tAcordando fila de suspensas pela task %d\n", out_task->id) ;
                #endif
                task_awake(
                        (task_t *) out_task->suspended_queue,
                        (task_t **) &out_task->suspended_queue) ;
            }
            out_task->status = TASK_TERMINADA ;
            user_tasks--;
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
    #ifdef DEBUG    
        printf("[task_setprio]\tTask %d recebeu prioridade %d\n", task->id, prio) ;
    #endif
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



/*
 * Retorna clock atual do sistema
 */
unsigned int systime(){
    return clock ;
}
//##############################################################################



/*
 * Suspende tarefa e insere em queue
 */
void task_suspend (task_t **queue){
    queue_remove((queue_t **) &task_queue, (queue_t *) out_task) ;
    out_task->status = TASK_SUSPENSA ;
    queue_append((queue_t **) queue, (queue_t *) out_task) ;
    task_switch(&dispatcher) ;
}
//##############################################################################



/*
 * Acorda tarefa e insere na fila de prontas
 */
void task_awake (task_t* task, task_t **queue){
    queue_remove((queue_t **) queue, (queue_t *) task) ;
    task->status = TASK_PRONTA ;
    queue_append((queue_t **) &task_queue, (queue_t *) task) ;
}
//##############################################################################



/*
 * Suspende task atual em função do parâmetro task recebido
 */
int task_wait (task_t* task){
    #ifdef DEBUG
        printf("[task_wait]\tTask %d esperará pelo fim de %d\n", out_task->id, task->id) ;
    #endif
    if(task->status == TASK_TERMINADA) return task->exit_code ;
    task_suspend((task_t **) &task->suspended_queue) ;
    return task->exit_code ;
}
//##############################################################################



/*
 * Coloca a tarefa na fila de tarefas dormindo
 */
void task_sleep(int t){
    #ifdef DEBUG
        printf("[task_sleep]\tTask %d dormirá por %d ms\n", out_task->id, t) ;
    #endif
    out_task->sleep_untill = systime() + t;
    task_suspend(&sleep_queue) ;
}
//##############################################################################



/*
 * Funções de sessão crítica
 */
void enter_cs (int *lock){ while(__sync_fetch_and_or (lock, 1)) ; }
void leave_cs (int *lock){ (*lock) = 0 ; }
//##############################################################################



/*
 * Inicia o semáforo
 */
int sem_init(semaphore_t *s, int value) {
    if(!s){
        printf("ERRO ao criar semáforo\n") ;
        return -1 ;
    }

    s->lock = 0 ;
    s->queue = NULL ;
    s->v = value ;
    s->valid = 1 ;
    #ifdef DEBUG
        printf("[sem_init]\tCriado semáforo com valor %d\n", value) ;
    #endif
    return 0 ;
}
//##############################################################################



/*
 * Requisita o semáforo
 */
int sem_down (semaphore_t *s){
    if(!s || !s->valid) return -1 ;
    #ifdef DEBUG
        printf("[sem_down]\tTarefa %d solicitou sem_down\n", task_id()) ;
    #endif

    enter_cs(&s->lock) ;
    s->v-- ;
    leave_cs(&s->lock) ;

    if (s->v < 0){
        #ifdef DEBUG
            printf("[sem_down]\tTask %d será supensa pelo semáforo\n", task_id()) ;
        #endif
        task_suspend(&s->queue) ;
    }
    return 0 ;
}
//##############################################################################



/*
 * Libera um semáforo
 */
int sem_up (semaphore_t *s){
    if(!s || !s->valid) return -1 ;

    enter_cs(&s->lock) ;
    s->v++ ;
    leave_cs(&s->lock) ;

    if(queue_size((queue_t *) s->queue) > 0){
        #ifdef DEBUG
            printf("[sem_up]\tTask %d será acordada pelo semáforo\n", s->queue->id) ;
        #endif
        task_awake((task_t *) s->queue, (task_t **) &s->queue) ;
    }

    return 0 ;
}
//##############################################################################



/*
 * Destroi o semáforo e acorda as tarefas
 */
int sem_destroy(semaphore_t *s){
    if(!s || !s->valid) return -1 ;

    while(s->queue){
        #ifdef DEBUG
            printf("[sem_destroy]\tTask %d será acordada pelo semáforo\n", s->queue->id) ;
        #endif

        task_awake((task_t *) s->queue, (task_t **) &s->queue) ;
    }
    s->valid = 0 ;
    return 0 ;
}
//##############################################################################



/*
 * Cria uma struct mqueue_t
 */
int mqueue_init(mqueue_t *queue, int max_msgs, int msg_size){
    if(!queue) return -1 ;

    queue->BUFF = malloc(max_msgs * msg_size) ;
    if(!queue->BUFF) return -1 ;
    
    queue->buff_top = 0;
    queue->msg_size = msg_size ;
    queue->valid = 1 ;

    if(sem_init(&queue->buff_s, 1))         return -1 ;
    if(sem_init(&queue->vaga_s, max_msgs))  return -1 ;
    if(sem_init(&queue->msgs_s, 0))         return -1;

    #ifdef DEBUG
        printf("[mqueue_init]\tCriada fila de mensagens com %d espaços de tamanho %d\n", max_msgs, msg_size) ;
    #endif
    
    return 0 ;
}
//##############################################################################



/*
 * Envia mensagem para a fila de mensagens
 */
int mqueue_send(mqueue_t *queue, void *msg){
    if(!queue || !queue->valid) return -1 ;

    sem_down(&queue->vaga_s) ;

    sem_down(&queue->buff_s) ;

    #ifdef DEBUG
        printf("[mqueue_send]\tTask %d envia mensagem à fila", task_id()) ;
    #endif

    int offset = queue->buff_top * queue->msg_size ;
    memcpy((void *) (queue->BUFF + offset), msg, queue->msg_size) ;
    queue->buff_top++ ;

    sem_up(&queue->buff_s) ;

    sem_up(&queue->msgs_s) ;

    return 0 ;
}
//##############################################################################



/*
 * Recebe uma mensagem da fila 
 */
int mqueue_recv(mqueue_t *queue, void *msg){
    if(!queue || !queue->valid) return -1 ;

    sem_down(&queue->msgs_s) ;

    sem_down(&queue->buff_s) ;

    #ifdef DEBUG
        printf("[mqueue_recv]\tTask %d solicita mensagem da fila\n", task_id());
    #endif

    memcpy(msg, (void *) queue->BUFF, queue->msg_size) ;
    memcpy((void *) queue->BUFF, (void *) (queue->BUFF + queue->msg_size), queue->msg_size * queue->buff_top) ;
    
    queue->buff_top-- ;

    sem_up(&queue->buff_s) ;

    sem_up(&queue->vaga_s) ;

    return 0 ;
}
//##############################################################################



/*
 * Encerra uma fila de mensagens
 */
int mqueue_destroy(mqueue_t *queue){
    if(!queue || !queue->valid) return -1 ;

    #ifdef DEBUG
        printf("[mqueue_destroy]\tDestruindo fila de mensagens\n") ;
    #endif

    if(sem_destroy(&queue->vaga_s)) return -1;
    if(sem_destroy(&queue->msgs_s)) return -1 ;
    if(sem_destroy(&queue->buff_s)) return -1 ;

    queue->valid = 0 ;
    free(queue->BUFF) ;

    return 0 ;
}
//##############################################################################



/*
 * Retorna o número de mensagens na fila
 */
int mqueue_msgs(mqueue_t *queue){
    if(!queue || !queue->valid) return -1 ;
    return queue->buff_top ;
}
//##############################################################################
