#include"common.h"


#define SCALE 10
struct alloc_tst{
    int sz;
    void* ptr;
} cases[SCALE] = {
    {}
};


void test_alloc(){
    for(int i = 0; i < SCALE; i++){
        cases[i].sz = rand() % 1024;
        cases[i].ptr = pmm->alloc(cases[i].sz);
    }
    for(int i = 0; i < SCALE; i++){
        printf("case %d, size : %d, ptr : %p\n", i, cases[i].sz, cases[i].ptr);
    }
}

int main(){
    pmm->init();
    test_alloc();

    return 0;
}