#include"test.h"
#include<memory.h>

#define STK_SZ 10000

extern void* STK[STK_SZ];
extern int top;

static typedef enum{
    ACT_ALLOC = 0, ACT_FREE = 1
} ACT; 
ACT rand_act(){
    return rand()%2;
}


#define max_alloc_sz 1024

void do_alloc(){
    if(FULL()){
        return;
    }
    void* ptr = pmm->alloc(rand()%(max_alloc_sz + 1));
    if(ptr == NULL){
        printf("alloc fails\n");
    }
    else{
        push_rand(ptr);
    }
}

void do_free(){
    if(EMPTY()){
        return;
    }
    void *freed = POP();
    pmm->free(freed);
}

void random_test(){
    INIT_STK();
    while(1){
        switch(rand_act()){
            case ACT_ALLOC:
                do_alloc();
                break;
            case ACT_FREE:
                do_free();
                break;
        }
    }
}



