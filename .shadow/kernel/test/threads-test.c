#include "test.h"
#include "thread.h"

static int cnt1 = 0;
#define NUM_THREADS 1


void Tclient(int tid){
    STK stk = { .top = -1 };
    printf("hello from thread %d \n", tid);
    while(cnt1 < SCALE){
        //printf("hello in %d \n", tid);
        switch (rand_act()){
            case ACT_ALLOC:
                do_alloc(&stk);
                break;
            case ACT_FREE:
                do_free(&stk);
                break;
        }
        if(++cnt1 == SCALE){
            break;
        }
        //if((cnt1) % NUM_REPORT == 0){ printf("%d acts done\n", NUM_REPORT); }
    }
    printf("thread %d has finished its job\n", tid);
}

void threads_test(){
    for(int i = 0; i < NUM_THREADS; i++){
        create(Tclient);
    }
    join();
}

