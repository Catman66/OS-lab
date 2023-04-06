#include "test.h"
#include <assert.h>

void copy_cases(){
    FILE * fp = fopen("../tmpt", "w");
    assert(fp != NULL);
    for(int i = 0; i < SCALE; i++){
        cases[i].sz = rand() % alloc_sz;
        cases[i].ptr = pmm->alloc(cases[i].sz);
        fprintf(fp, "%p %d\n", cases[i].ptr, cases[i].sz);   
    }
    fclose(fp);
}

void print_cases(){
    for(int i = 0; i < SCALE; i++){
        printf("case : sz: %d, ptr : %p\n", cases[i].sz, cases[i].ptr);
    }
}

void sort_cases(){
    for(int i = 0; i < SCALE - 1; i++){
        for(int j = 0; j < SCALE - 1 - i; j++){
            if(cases[j].ptr > cases[j+1].ptr){
                struct alloc_tst tmpt = cases[j];
                cases[j] = cases[j+1];
                cases[j+1] = tmpt;
            }
        }
    }
    for(int i = 0; i < SCALE - 1; i++){
        if(!(cases[i].ptr < cases[i+1].ptr)){
            printf("%d larger than %d\n", i, i + 1 );
            print_cases();
            assert(0);
        }
    }
}


void do_alloc(){
    for(int i = 0; i < SCALE; i++){
        cases[i].sz = rand() % alloc_sz;
        cases[i].ptr = pmm->alloc(cases[i].sz); 
    }
    sort_cases();
    copy_cases();
}



void check_overlap(){
    for(int i = 1; i < SCALE; i++){
        if(END_SP(cases[i-1]) > START_SP(cases[i])){
            printf("overlapping error between: %d and %d\n", i-1, i);
            fflush(stdout);
            assert(0);
        }
    }
    printf("check overlap passed\n");
}

void check_in_heap(){
    for(int i = 0; i < SCALE; i++){
        if(!ALLOC_FAIL(cases[i]) && START_SP(cases[i]) < heap.start && END_SP(cases[i]) >= heap.end){
            printf("out of heap error : %d \n", i);
            fflush(stdout);
            assert(0);
        }
    }
    printf("check in heap passed\n");

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
        uintptr_t ptr = (uintptr_t)(cases[i].ptr);

        if(!ALLOC_FAIL(cases[i]) && ((ptr & mask) != 0)){
            printf("alignment error : %d \n", i);
        }
    }
    printf("check alignment passed\n");

}

void check(){
    do_alloc();
    check_overlap();
    check_in_heap();
    check_align();
}

int main(){
    pmm->init();
    check();

    return 0;
}