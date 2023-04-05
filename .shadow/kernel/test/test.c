#include"common.h"

void test_alloc(){
    void* p[10];
    for(int i = 0; i < 10; i++){
        p[i] = pmm->alloc(rand()%1024);
    }
    for(int i = 0; i < 10; i++){
        printf("alloc %d : %p\n", i, p[i]);
    }
}

int main(){
    pmm->init();
    test_alloc();

    return 0;
}