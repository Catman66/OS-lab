#include "test.h"
#include <assert.h>

void print_cases(FILE* fp){
    assert(fp != NULL);
    for(int i = 0; i < SCALE; i++){
        fprintf(fp, "%p %d\n", cases[i].ptr, cases[i].sz);   
    }
}

void copy_cases(){
    FILE* fp = fopen("tmpt", "w");
    print_cases(fp);
    fclose(fp);
}

void sort_cases(){
    for(int i = 0; i < SCALE - 1; i++){
        for(int j = 0; j < SCALE - 1; j++){
            if(cases[j].ptr > cases[j+1].ptr){
                alloc_tst tmpt = cases[j];
                cases[j] = cases[j+1];
                cases[j+1] = tmpt;
            }
        }
    }
}

void do_alloc(){
    for(int i = 0; i < SCALE; i++){
        cases[i].sz = rand() % alloc_sz;
        cases[i].ptr = pmm->alloc(cases[i].sz); 
    }
    
    //copy_cases();
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

int num_bit_set(int sz){
    sz--;
    int ret = 0;
    while(sz > 1){
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
        int align_bit = num_bit_set(cases[i].sz);
        uintptr_t mask = make_mask(align_bit);
        uintptr_t ptr = (uintptr_t)(cases[i].ptr);

        if(!ALLOC_FAIL(cases[i]) && ((ptr & mask) != 0)){
            printf("alignment error : case %d(%p, %x)\n", i, cases[i].ptr, cases[i].sz);
        }
    }
    printf("check alignment passed\n");
}

void check(){
    do_alloc();
    printf("alloc finished\n");
    sort_cases();
    printf("sort finished\n");
    check_overlap();
    check_in_heap();
    check_align();
    printf("hello\n");
}


int main(){
    pmm->init();
    check();
    return 0;
}