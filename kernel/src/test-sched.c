#include <os.h>
#include <common.h>

void Tsilent(void* arg){
    int i = 0;
    while(1){
        i++;
    }
    printf("%d\n", i);
}

void test_sched(){
    for(int i = 0; i < 8; i++){
        kmt->create(tsk_alloc(), "T", Tsilent, NULL);
    }
}


task_t* tsk_alloc(){
    return pmm->alloc(sizeof(task_t));
}