#include"test.h"
#include<memory.h>

static STK stk01 = { .top = -1 };

//1/8 portion of alloc is in size page
size_t alloc_sz(){
    size_t sz = rand_alloc_sz();
    if(sz % 8 == 0){
        return 4096;
    }
    return sz;
}

void do_alloc(STK* stk){
    if(full(stk)){
        return;
    }
    
    void* ptr = pmm->alloc(alloc_sz());
    if(ptr == NULL){
        printf("alloc fails\n");
    }
    else{      
        rand_push(stk, ptr);
    }
}
void do_free(STK* stk){
    if(empty(stk)){
        return;
    }
    void *freed = pop(stk);
    pmm->free(freed);
}

int cnt_succ_act = 0;
void random_test(){
    while(1){
        switch(rand_act()){
            case ACT_ALLOC:
                do_alloc(&stk01);
                break;
            case ACT_FREE:
                do_free(&stk01);
                break;
        }
        cnt_succ_act++;
        //  printf("%d succ\n", cnt_succ_act);
        if(cnt_succ_act % 10000 == 0){
            printf("10000 succ acts\n");
        }
    }
}

bool empty(STK* stk){  return stk->top == -1; }
bool full(STK* stk) {  return stk->top == STK_SZ - 1; }
void rand_push(STK* stk, void* ptr){
    if(empty(stk)){
        stk->content[++(stk->top)] = ptr;
        return;
    }
    int rand_idx = rand() % (stk->top + 1);
    stk->content[++(stk->top)] = stk->content[rand_idx];
    stk->content[rand_idx] = ptr;
}
void *pop(STK* stk){
    return stk->content[stk->top--];
}

ACT     rand_act()      {  return rand()%2; }
size_t  rand_alloc_sz() {  return rand() % MAX_ALLOC_SZ + 1; }

