#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include "ppos_data.h"
#include "ppos_disk.h"
#include "ppos.h"
#include "hard_disk.h"
#include "queue.h"

struct sigaction disk_action ;      // Tratador de sinais de disco
disk_t disk_components ;            // Struct de controle
task_t disk_task ;                  // Task do gerenciador de disco

/*
 * Corpo da tarefa driver de disco
 */
void diskDriverBody (void *args){
    for(;;){
        sem_down(&disk_components.disk_s) ; // Solicita exclusividade ao disco

        // Se foi acordado por sinal do disco, acorda tarefa que solicitou
        if(disk_components.signal) {
            task_awake(
                    (task_t *) disk_components.disk_suspend,
                    (task_t **) &disk_components.disk_suspend
            ) ;
            disk_components.signal = 0 ;
        }

        // Se o disco está livre e há requisições na fila
        if(disk_cmd(DISK_CMD_STATUS, 0, 0) == DISK_STATUS_IDLE && queue_size((queue_t *) disk_components.reqs) >0){
            req_t *aux = disk_components.reqs ;
            #ifdef DEBUG
                printf("[diskDriverBody]\tRequisição de %s no bloco %d encontrada\n\t\t\tBUFFER: %s\n",
                    (aux->op) ? "leitura" : "escrita",
                    aux->blk,
                    (char *) aux->buff
                    ) ;
            #endif

            if(aux->op){ // op == 1 LEITURA
                disk_cmd(DISK_CMD_READ, aux->blk, aux->buff) ;
                #ifdef DEBUG
                    printf("[DiskDriverBody]\tRequisição de leitura enviada ao disco\n") ;
                #endif
            } else { // op == 0 ESCRITA
                disk_cmd(DISK_CMD_WRITE, aux->blk, aux->buff) ;
                #ifdef DEBUG
                    printf("[DiskDriverBody]\tRequisição de escrita enviada ao disco\n") ;
                #endif
            }
            // Remove solicitação da fila
            queue_remove((queue_t **) &disk_components.reqs, (queue_t *) aux) ;
        }

        sem_up(&disk_components.disk_s) ; // Libera o disco

        task_suspend((task_t **) &disk_components.disk_suspend) ;
    }
}
//##############################################################################



/*
 * Tratador de sinais SIGUSR1
 */
void disk_handler(){
    disk_components.signal = 1 ;
    #ifdef DEBUG
        printf("[disk_handler]\tRECEBI SINAL SIGUSR1\n") ;
    #endif
    task_awake((task_t *) &disk_task, (task_t **) &disk_components.disk_suspend) ;
}
//##############################################################################



/*
 * Inicializa a struct do disco
 */
int disk_mgr_init (int *numBlocks, int *blockSize){
    // Inicializa disco
    if(disk_cmd(DISK_CMD_INIT, 0, 0)){
        printf("[disk_mgr_init]\tFalha ao inicializar disco\n") ;
        return -1;
    }

    // Preenche as variáveis de informação do disco
    *(numBlocks) = disk_cmd(DISK_CMD_DISKSIZE, 0, 0) ;
    *(blockSize) = disk_cmd(DISK_CMD_BLOCKSIZE, 0, 0) ;

    // Inicializa tratador de sinais pro sinal SIGUSR1
    disk_action.sa_handler = disk_handler ;
    sigemptyset(&disk_action.sa_mask) ;
    disk_action.sa_flags = 0 ;
    if(sigaction(SIGUSR1, &disk_action, 0) < 0){
        perror("ERRO em sigaction: ") ;
        exit(1) ;
    }
    #ifdef DEBUG
        printf("[disk_mgr_init]\tCriado tratador de sinais do disco\n") ;
    #endif

    // Inicializa a struct de controle e a tarefa do disco
    disk_components.reqs = NULL ;
    disk_components.disk_suspend = NULL ;
    sem_init(&disk_components.disk_s, 1) ;
    set_sys_flag() ;
    task_init(&disk_task, diskDriverBody, NULL) ;
    #ifdef DEBUG
        printf("[disk_mgr_init]\tNovo tratador de disco inicializado\n") ;
    #endif

    return 0 ;
}
//##############################################################################



/*
 * Faz uma requisição
 */
int add_req(int blk, void* bff, int op){
    // Aloca e preenche nova requisição
    req_t *r = (req_t *) malloc(sizeof(req_t)) ;
    if(!r) return -1 ;

    r->op = op ;
    r->blk = blk ;
    r->buff = bff ;
    r->next = r->prev = NULL ;

    #ifdef DEBUG
        printf("[add_req]\tRequisição de %s registrada para bloco %d\n",
            (r->op) ? "leitura" : "escrita",
            r->blk
        ) ;
    #endif

    sem_down(&disk_components.disk_s) ; // Solicita exclusividade ao disco

    // Adiciona à fila de requisições e acorda a tarefa do disco se preciso
    queue_append((queue_t **) &disk_components.reqs, (queue_t *) r);

    if(disk_task.status == TASK_SUSPENSA)
        task_awake((task_t *) &disk_task, (task_t **) &disk_components.disk_suspend) ;

    sem_up(&disk_components.disk_s) ; // Libera semáforo do disco

    task_suspend((task_t **) &disk_components.disk_suspend) ;

    return 0 ;
}
//##############################################################################



/*
 * Lê bloco do disco
 */
int disk_block_read (int block, void *buffer) {
    #ifdef DEBUG
        printf("[disk_block_read]\tChamada por tarefa %d\n", task_id()) ;
    #endif
    return add_req(block, buffer, 1) ;
}
//##############################################################################



/*
 * Escreve bloco no disco
 */
int disk_block_write (int block, void *buffer) {
    #ifdef DEBUG
            printf("[disk_block_write]\tChamada por tarefa %d\n", task_id()) ;
    #endif
    return add_req(block, buffer, 0) ;
}
//##############################################################################
