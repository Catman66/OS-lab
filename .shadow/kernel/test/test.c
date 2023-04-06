#include "test.h"
#include <assert.h>

void do_alloc(){
    for(int i = 0; i < SCALE; i++){
        //scanf("%d", &(cases[i].sz));
        cases[i].sz = rand() % alloc_sz;
        cases[i].ptr = pmm->alloc(cases[i].sz);
    }
}

void check_overlap(){
    for(int i = 1; i < SCALE; i++){
        if(END_SP(cases[i-1]) > START_SP(cases[i])){
            printf("overlapping error between: %d and %d\n", i-1, i);
            fflush(stdout);
            assert(0);
        }
    }
}

void check_in_heap(){
    for(int i = 0; i < SCALE; i++){
        if(!ALLOC_FAIL(cases[i]) && START_SP(cases[i]) < heap.start && END_SP(cases[i]) >= heap.end){
            printf("out of heap error : %d \n", i);
            fflush(stdout);
            assert(0);
        }
    }
}

int align_bit_of(int sz){
    int ret = 0;
    while(sz > 0){
        sz >>= 1;
        ret++;
    }
    return ret;
}
uintptr_t make_mask(int align_bit){
    return ~(UINTPTR_MAX << align_bit);
}

void check_align(){
    for(int i = 0; i < SCALE; i++){
        int align_bit = align_bit_of(cases[i].sz);
        uintptr_t mask = make_mask(align_bit);
        uintptr_t ptr = uintptr_t(cases[i].ptr);

        if(!ALLOC_FAIL(cases[i]) && ((ptr & mask) != 0)){
            printf("alignment error : %d \n", i);
        }
    }
}

void check(){
    do_alloc();
    check_overlap();
    check_in_heap();
    check_align();
}

int main(){
    pmm->init();
    do_alloc();

    return 0;
}