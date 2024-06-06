// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022

// interface do gerente de disco rígido (block device driver)

#ifndef __DISK_MGR__
#define __DISK_MGR__

// estruturas de dados e rotinas de inicializacao e acesso
// a um dispositivo de entrada/saida orientado a blocos,
// tipicamente um disco rigido.

// Estrutura para armazenar as solicitações de disco
typedef struct req_t
{
    struct req_t *prev, *next ;     // Ponteiros de enfileiramento
    int op ;                        // 0 para escrita, 1 para leitura 
    void *buff ;                    // Buffer da operação
    int blk ;                       // Bloco 
} req_t ; 

// estrutura que representa um disco no sistema operacional
typedef struct
{
    int signal ;            // Diz se tarefa foi acordada pelo sinal do disco:
    semaphore_t disk_s ;    // semáforo para acesso ao disco
    task_t *disk_suspend ;  // Fila de tarefas suspensas pelo disco
    req_t *reqs ;           // Fila de requisições
} disk_t ;

// inicializacao do gerente de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: tamanho do disco, em blocos
// blockSize: tamanho de cada bloco do disco, em bytes
int disk_mgr_init (int *numBlocks, int *blockSize) ;

// leitura de um bloco, do disco para o buffer
int disk_block_read (int block, void *buffer) ;

// escrita de um bloco, do buffer para o disco
int disk_block_write (int block, void *buffer) ;

#endif
