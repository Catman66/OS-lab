#include "test.h"
#include "thread.h"

void Tclient(int tid){
    STK stk;
    printf("hello from thread %d \n", tid);
    while(1){
        printf("hello in\n");
        switch (rand_act()){
            case ACT_ALLOC:
                do_alloc(&stk);
                break;
            case ACT_FREE:
                do_free(&stk);
                break;
        }
        printf("hello out\n");
    }
}

void threads_test(){
    for(int i = 0; i < 4; i++){
        create(Tclient);
    }
    join();
}

