#include <os.h>
#include<common.h>

#define NThread 4
volatile int s = 0; 
void Tsum(){
    for(int i = 0; i < 10000; i++){
        s++;
    }
}
void test_sum(){
    for(int i = 0; i < NThread; i++){
        kmt->create(pmm->alloc(sizeof(task_t)), "Tsum", Tsum, NULL);
    }
}