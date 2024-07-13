// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Teste de filas de mensagens

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "ppos.h"
#include "ppos_disk.h"

int main (int argc, char *argv[]) {
    ppos_init () ;

    int numblocks, blocksize ;

    disk_mgr_init(&numblocks, &blocksize) ;
    printf("O disco tem %d blocos de %d bytes\n", numblocks, blocksize) ;

    char buff[blocksize] ;
    char saida[blocksize] ;

    strcpy(buff, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789<") ;
    disk_block_write(0, (void*) buff) ;
    printf("Main saiu de suspensão\n") ;

    disk_block_read(0, (void *) saida) ;
    printf("Main saiu de suspensão\n") ;
    printf("%s\n", saida) ;

    task_exit (0) ;
}
