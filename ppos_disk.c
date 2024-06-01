#include <stdio.h>
#include "ppos_disk.h"
#include "hard_disk.h"

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
    
    #ifdef DEBUG
        printf("[disk_mgr_init]\tNovo tratador de disco inicializado\n") ;
    #endif

    return 0 ;
}
//##############################################################################



/*
 * Lê bloco do disco
 */
int disk_block_read (int block, void *buffer) ;
//##############################################################################



/*
 * Escreve bloco no disco
 */ 
int disk_block_write (int block, void *buffer) ;
//##############################################################################
