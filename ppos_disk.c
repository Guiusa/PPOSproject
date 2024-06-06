#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include "ppos_data.h"
#include "ppos_disk.h"
#include "ppos.h"
#include "hard_disk.h"

struct sigaction disk_action ;      // Tratador de sinais de disco
disk_t disk_components ;
task_t disk_task ;

void diskDriverBody (void *args){
    for(;;){
        sem_down(&disk_components.disk_s) ;

        if(disk_components.signal) {
            task_awake(
                    (task_t *) disk_components.disk_suspend,
                    (task_t **) &disk_components.disk_suspend
            ) ;
            disk_components.signal = 0 ;
        }

        if(disk_cmd(DISK_CMD_STATUS, 0, 0) > 1 && disk_components.reqs != 0){
            req_t *aux = disk_components.reqs ;
            if(aux->op == 0){
                disk_cmd(DISK_CMD_WRITE, aux->blk, aux->buff) ;
            } else {
                disk_cmd(DISK_CMD_READ, aux->blk, aux->buff) ;
            }
            queue_remove((queue_t **) disk_components.reqs, (queue_t *) aux) ;
            free(aux) ;
        }

        sem_up(&disk_components.disk_s) ;

        task_suspend((task_t **) &disk_components.disk_suspend) ;

    }
}

void disk_handler(){
    disk_components.signal = 1 ;
    task_awake((task_t *) &disk_task, (task_t **) &disk_components.disk_suspend) ;
}

/*
 * Inicializa a struct do disco
 */
int disk_mgr_init (int *numBlocks, int *blockSize){
    if(disk_cmd(DISK_CMD_INIT, 0, 0)){
        printf("[disk_mgr_init]\tFalha ao inicializar disco\n") ;
        return -1;
    }

    *(numBlocks) = disk_cmd(DISK_CMD_DISKSIZE, 0, 0) ;
    *(blockSize) = disk_cmd(DISK_CMD_BLOCKSIZE, 0, 0) ;
    
    disk_action.sa_handler = disk_handler ;
    sigemptyset(&disk_action.sa_mask) ;
    disk_action.sa_flags = 0 ;
    if(sigaction(SIGUSR1, &disk_action, 0) < 0){
        perror("ERRO em sigaction: ") ;
        exit(1) ;
    }

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
 * Lê bloco do disco
 */
int disk_block_read (int block, void *buffer) {
    req_t *r = (req_t *) malloc(sizeof(req_t)) ;
    r->op = 1 ;
    r->blk = block ;
    r->buff = buffer ;

    sem_down(&disk_components.disk_s) ;

    queue_append((queue_t **) &disk_components.reqs, (queue_t *) r);

    if(disk_task.status == TASK_SUSPENSA)
        task_awake((task_t *) &disk_task, (task_t **) &disk_components.disk_suspend) ;

    sem_up(&disk_components.disk_s) ;

    task_suspend((task_t **) &disk_components.disk_suspend) ;

    return 0 ;
}
//##############################################################################



/*
 * Escreve bloco no disco
 */ 
int disk_block_write (int block, void *buffer) {
    req_t *r = (req_t *) malloc(sizeof(req_t)) ;
    r->op = 0 ;
    r->blk = block ;
    r->buff = buffer ;

    sem_down(&disk_components.disk_s) ;

    queue_append((queue_t **) &disk_components.reqs, (queue_t *) r);

    if(disk_task.status == TASK_SUSPENSA)
        task_awake((task_t *) &disk_task, (task_t **) &disk_components.disk_suspend) ;

    sem_up(&disk_components.disk_s) ;

    task_suspend((task_t **) &disk_components.disk_suspend) ;

    return 0 ;
}
//##############################################################################
