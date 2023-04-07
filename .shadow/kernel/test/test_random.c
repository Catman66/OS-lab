#include"test.h"
#include<memory.h>

#define STK_SZ 10000

static void* STK[STK_SZ];
int top = -1;

#define EMPTY()         (top == -1)
#define FULL()          (top == STK_SZ - 1)
#define PUSH(ptr)       (STK[++top] = (ptr))
#define POP()           (STK[top--])
#define INIT_STK()      (top = -1)

void push_rand(void* ptr){
    int idx = rand() % (top + 1);
    STK[++top] = STK[idx];
    STK[idx] = ptr;
}

typedef enum{
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
    while(1){
        printf("hello\n");
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



