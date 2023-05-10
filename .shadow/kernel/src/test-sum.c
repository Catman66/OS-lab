#include <os.h>
#include<common.h>

#define NThread 8
#define ADDED 10000
static volatile int s_nlk = 0, s_lk = 0;

static spinlock_t lk;

void Tsum(void* name){
    for(int i = 0; i < 100; i ++){
        kmt->spin_lock(&lk);
        s_nlk++;
        kmt->spin_unlock(&lk);
    }
    printf("%s finished, res:%d\n", *(const char *)name, s_nlk);
    while(1){
        ;
    }
}

const char * thread_names[NThread] = {
    "T1", "T2", "T3", "T4", "T5", "T6", "T7", "T8"
};

void test_sum(){
    for(int i = 0; i < NThread; i++){
        kmt->create(pmm->alloc(sizeof(task_t)), thread_names[i], Tsum, (void*)thread_names[i]);
    }
}