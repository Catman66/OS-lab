#include <os.h>
#include<common.h>

#define NThread 4
static volatile int s = 0; 
void Tsum(){
    for(volatile int i = 0; i < 10000; i++){
        //
        s++;
    }
    printf("current sum : %d\n", s);
    while(1){
        ;//yield();
    }
}
void test_sum(){
    for(int i = 0; i < NThread; i++){
        kmt->create(pmm->alloc(sizeof(task_t)), "Tsum", Tsum, NULL);
    }
}