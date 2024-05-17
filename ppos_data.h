// GRR20210572 Guiusepe Oneda Dal Pai
// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto
#include "queue.h"          // 
// Status de tarefas
#define TASK_TERMINADA   0
#define TASK_PRONTA      1
#define TASK_RODANDO     2
#define TASK_SUSPENSA    3

// Prioridades
#define PRIO_DEFAULT 0
#define PRIO_ALTA   -20
#define PRIO_BAIXA  20
#define PRIO_PASSO   -1

// Tamanho stack
#define TASK_STACK_SIZE 64*1024

// Quantum que cada tarefa recebe - 60 ms
#define QUANTUM 20
// Simulação do tick do hardware a cada 1 ms
#define FIRST_TICK_S 0 
#define INTERVAL_TICK_S 0
#define FIRST_TICK_US 1000
#define INTERVAL_TICK_US 1000
                                    
// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
  struct task_t *prev, *next ;		// ponteiros para usar em filas
  int id ;				            // identificador da tarefa
  ucontext_t context ;			    // contexto armazenado da tarefa
  short status ;			        // pronta, rodando, suspensa, ... 
  short quantum ;                   // ticks pra executar
  short sys_task ;                  // sistema ou não
  int prio_s ;                      // prioridade [-20, 20]
  int prio_d ;                      // prioridade dinâmica
  int ativ ;                        // numero de ativações
  unsigned int first_clock ;        // primeiro clock da tarefa
  int cpu_time ;                    // tempo com cpu
  queue_t** suspended_queue ;        // fila de tarefas suspensas por ela
  // ... (outros campos serão adicionados mais tarde)
} task_t ;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t ;

#endif
