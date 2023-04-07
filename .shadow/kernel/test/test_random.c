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
    if(EMPTY()){
        PUSH(ptr);
        return;
    }
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
    size_t sz = rand();
    sz %= max_alloc_sz;
    void* ptr = pmm->alloc(sz);
    
    if(ptr == NULL){
        printf("alloc fails\n");
    }
    else{      
        printf("allocated : %p\n", ptr);  
        push_rand(ptr);
    }
}

void do_free(){
    if(EMPTY()){
        return;
    }
    void *freed = POP();
    printf("freeing %p\n", freed);
    pmm->free(freed);
}

int cnt_succ_act = 0;
void random_test(){
    while(1){
        switch(rand_act()){
            case ACT_ALLOC:
                do_alloc();
                break;
            case ACT_FREE:
                do_free();
                break;
        }
        cnt_succ_act++;
        printf("%d succ\n", cnt_succ_act);
        // if(cnt_succ_act % 1000000 == 0){
        //     printf("1000000 succ acts\n");
        // }
    }
}



