#include "test.h"
#include <assert.h>
#include <time.h>

int main(){
    pmm->init();
    printf("hello\n");
    //check();
    random_test();
    return 0;
}

struct alloc_tst{
    int sz;
    void* ptr;
} cases[SCALE] = {
    {}
};
typedef struct alloc_tst alloc_tst;

#define END_SP(c) ((c).ptr + (c).sz)
#define START_SP(c) ((c).ptr)
#define ALLOC_FAIL(c) ((c).ptr == NULL)
void do_all_alloc();
void check_overlap();
void check_in_heap();
void check_align();

void check();

void print_cases(FILE* fp){
    assert(fp != NULL);
    for(int i = 0; i < SCALE; i++){
        fprintf(fp, "%p 0x%x\n", cases[i].ptr, cases[i].sz);   
    }
}

void copy_cases_totmpt(){
    FILE* fp = fopen("tmpt", "w");
    assert(fp != NULL);
    print_cases(fp);
    fclose(fp);
}




void swap(alloc_tst* c1, alloc_tst* c2){
    alloc_tst tmpt = *c1;
    *c1 = *c2;
    *c2 = tmpt;
}

void sort_cases(){
    for(int i = 0; i < SCALE - 1; i++){
        for(int j = 0; j < SCALE - 1; j++){
            if(cases[j].ptr > cases[j+1].ptr){
                swap(&cases[j], &cases[j+1]);
            }
        }
    }
}



void do_all_alloc(){
    printf("SCALE : %d\n", SCALE);
    srand((unsigned int)time(NULL));

    int cnt = 0;
    for(int i = 0; i < SCALE; i++){
        cases[i].sz = rand() % alloc_sz;
        cnt += ((cases[i].ptr = pmm->alloc(cases[i].sz)) != NULL);  
    }
    printf("successfully allocted: %d\n", cnt);
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


void shuffle_cases(){
    for(int i = 0; i < SCALE; i++){
        int i1 = rand() % SCALE, i2 = rand()%SCALE;
        swap(&cases[i1], &cases[i2]);
    }
}

void free_all(){
    for(int i = 0;i < SCALE; i++){
        pmm->free(cases[i].ptr);
    }
}
void check(){
    do_all_alloc();
    sort_cases();
    copy_cases_totmpt();
    check_overlap();
    check_in_heap();
    check_align();
    shuffle_cases();
    fflush(stdout);
    free_all();
    
}


