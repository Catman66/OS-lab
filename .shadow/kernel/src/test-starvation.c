#include <common.h>
#include <os.h>


void print_self(void * arg){
    while(1){
        putch(*(char*)arg);
    }
}

#define Nthread 8
void test_starvation(){
    for(int i = 0; i < Nthread; i++){
        kmt->create(pmm->alloc(sizeof(task_t)), "printer", print_self, "abcdefgh"+i);
    }
}