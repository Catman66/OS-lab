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
        scanf("%d", &(cases[i].sz));
        cases[i].ptr = pmm->alloc(cases[i].sz);
    }
    printf("%d\n", SCALE);

    for(int i = 0; i < SCALE; i++){
        printf("%p %d\n", cases[i].ptr, cases[i].sz);
    }
}

int main(){
    pmm->init();
    test_alloc();

    return 0;
}