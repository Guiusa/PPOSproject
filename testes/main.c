#include <stdio.h>
#include "ppos.h"

mqueue_t msgs ;
char rcv[1] ;
int main(){
    ppos_init() ;

    mqueue_init(&msgs, 5, sizeof(char)) ;

    for(int i = 0; i<4; i++){
        mqueue_send(&msgs, (void *) "b") ;
        printf("%s\n", (char *) msgs.BUFF) ;
    }

    for(int i = 0; i<5; i++){
        mqueue_recv(&msgs, (void *) &rcv) ;
        printf("%s\n%s\n",
                (char *) msgs.BUFF,
                rcv
        ) ;
    }

    mqueue_destroy(&msgs) ;

    task_exit(0) ;
}
