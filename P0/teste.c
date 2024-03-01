// 
// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022
// Teste da implementação de fila genérica queue.c/queue.h.

// ESTE ARQUIVO NÃO DEVE SER MODIFICADO - ELE SERÁ SOBRESCRITO NOS TESTES

// operating system check
#if defined(_WIN32) || (!defined(__unix__) && !defined(__unix) && (!defined(__APPLE__) || !defined(__MACH__)))
#warning Este código foi planejado para ambientes UNIX (LInux, *BSD, MacOS). A compilação e execução em outros ambientes é responsabilidade do usuário.
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "queue.h"

#define N 100

// A estrutura "filaint_t" será usada com as funções de queue.c usando um
// casting para o tipo "queue_t". Isso funciona bem, se os campos iniciais
// de ambas as estruturas forem os mesmos. De acordo com a seção 6.7.2.1 do
// padrão C99: "Within a structure object, the non-bit-ﬁeld members and the
// units in which bit-ﬁelds reside have addresses that increase in the order
// in which they are declared.".

typedef struct filaint_t
{
   struct filaint_t *prev ;  // ptr para usar cast com queue_t
   struct filaint_t *next ;  // ptr para usar cast com queue_t
   int id ;
   // outros campos podem ser acrescidos aqui
} filaint_t ;

filaint_t item[N];
filaint_t *fila0, *fila1, *aux, *final ;
int ret ;

//------------------------------------------------------------------------------

// imprime na tela um elemento da fila (chamada pela função queue_print)
void print_elem (void *ptr)
{
   filaint_t *elem = ptr ;

   if (!elem)
      return ;

   elem->prev ? printf ("%d", elem->prev->id) : printf ("*") ;
   printf ("<%d>", elem->id) ;
   elem->next ? printf ("%d", elem->next->id) : printf ("*") ;
}

//------------------------------------------------------------------------------

// retorna 1 se a estrutura da fila está correta, 0 senão
int fila_correta (filaint_t *fila)
{
   filaint_t *aux ;

   // uma fila vazia sempre está correta
   if (!fila)
      return 1 ;

   // fila com um só elemento e correta
   if ((fila->next == fila) && (fila->prev == fila))
      return 1 ;

   // fila com um só elemento, mas incorreta
   if ((fila->next == fila) || (fila->prev == fila))
   {
      printf ("ERRO: ponteiros errados na fila com um elemento\n") ;
      return 0 ;
   }

   // fila com mais elementos, percorrer e testar todos os ponteiros
   aux = fila ;
   do
   {
      // testa ponteiro next (avaliação em curto-circuito)
      if (aux->next && (aux->next->prev == aux))
         ; // ponteiro ok
      else
      {
         printf ("ERRO: ponteiros errados ->next ou ->next->prev\n") ;
         return 0 ;
      }

      // testa ponteiro prev (avaliação em curto-circuito)
      if (aux->prev && (aux->prev->next == aux))
         ; // ponteiro ok
      else
      {
         printf ("ERRO: ponteiros errados ->prev ou ->prev->next\n") ;
         return 0 ;
      }
      aux = aux->next ;
   }
   while (aux != fila) ;

   // passou por tudo, estrutura da fila parece estar ok
   return 1 ;
}

//------------------------------------------------------------------------------

int main (int argc, char **argv, char **envp)
{
   int i ;

   // inicializa os N elementos
   for (i=0; i<N; i++)
   {
      item[i].id = i ;
      item[i].prev = NULL ;
      item[i].next = NULL ;
   }

   // PARTE 1: queue_append e queue_size =======================================

   // Teste: inserir N elemementos na fila e verificar a estrutura
   printf ("Testando insercao de %d elementos...\n", N) ;
   fila0 = NULL ;
   for (i=0; i<N; i++)
   {
      assert (queue_size ((queue_t*) fila0) == i) ;
      queue_append ((queue_t **) &fila0, (queue_t*) &item[i]) ;
      assert (fila_correta (fila0)) ;
   }

   // Teste: contar o numero de elementos na fila e verificar a ordem
   // dos elementos inseridos
   printf ("Testando tamanho da fila e ordem dos %d elementos...\n", N) ;
   aux = fila0 ;
   i=0 ;
   do
   {
      assert (i == aux->id) ; // testa posição do elemento i
      i++ ;
      aux = aux->next ;
   } while (aux != fila0) ;

   assert (i == N) ;

   assert (queue_size ((queue_t*) fila0) == N) ;

   printf ("Testes de insercao funcionaram!\n") ;

}
