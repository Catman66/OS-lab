#include <os.h>
#include<common.h>

#define NThread 4
static volatile int s_nlk = 0, s_lk = 0; 
extern spinlock_t usr_lk;

void Tsum(){
    for(int i = 0; i < 100000000; i++){
        s_nlk++;
    }
    printf("without final sum: %d\n", s_nlk);
    for(int i = 0; i < 100000000; i++){
        kmt->spin_lock(&usr_lk);
        s_lk++;
        kmt->spin_unlock(&usr_lk);
    }
    printf("with lock, final sum: %d \n", s_lk);
}
void test_sum(){
    for(int i = 0; i < NThread; i++){
        kmt->create(pmm->alloc(sizeof(task_t)), "Tsum", Tsum, NULL);
    }
}