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
        scanf("%d", cases[i].sz);
        cases[i].ptr = pmm->alloc(cases[i].sz);
    }
    for(int i = 0; i < SCALE; i++){
        printf("%p", cases[i].ptr);
    }
}

int main(){
    pmm->init();
    test_alloc();

    return 0;
}