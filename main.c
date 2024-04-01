// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Teste do task dispatcher e escalonador FCFS

#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"

task_t Pang, Peng, Ping, Pong, Pung ;

// corpo das threads
void Body (void * arg)
{
   int i ;

   printf ("%s: inicio\n", (char *) arg) ;
   for (i=0; i<5; i++)
   {
      printf ("%s: %d\n", (char *) arg, i) ;
      task_yield ();
   }
   printf ("%s: fim\n", (char *) arg) ;
   task_exit (0) ;
}

int main (int argc, char *argv[])
{
   printf ("main: inicio\n");

   ppos_init () ;
   
   task_init (&Pang, Body, "    Pang") ;
   task_init (&Peng, Body, "        Peng") ;
   task_init (&Ping, Body, "            Ping") ;
   task_init (&Pong, Body, "                Pong") ;
   task_init (&Pung, Body, "                    Pung") ;
   task_setprio(&Pang, -20) ;
   task_setprio(&Peng, -10) ;
   task_setprio(&Ping, 0) ;
   task_setprio(&Pong, 10) ;
   task_setprio(&Pung, 20) ;

   printf("Pang: %d\n", task_getprio(&Pang));
   printf("Peng: %d\n", task_getprio(&Peng));
   printf("Ping: %d\n", task_getprio(&Ping));
   printf("Pong: %d\n", task_getprio(&Pong));
   printf("Pung: %d\n", task_getprio(&Pung));

   printf ("main: fim\n");
   
   task_exit (0);
}
