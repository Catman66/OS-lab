#include"common.h"


#define SCALE 10
struct alloc_tst{
    int sz;
    void* reslt;
} cases[SCALE] = {
    {}
};


void test_alloc(){
    for(int i = 0; i < SCALE; i++){
        cases[i].sz = rand();
        cases[i].reslt = pmm->alloc(cases[i].sz);
    }
}

int main(){
    pmm->init();
    test_alloc();

    return 0;
}