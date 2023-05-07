#include <os.h>
#include<common.h>

#define NThread 4
#define ADDED 10000
static volatile int s_nlk = 0, s_lk = 0; 
extern spinlock_t usr_lk;

void Tsum(){
    printf("intr: %d\n", ienabled());
    printf("num cpu: %d\n", cpu_count());
    for(int i = 0; i < ADDED; i++){
        s_nlk++;
        printf("%d", s_nlk);
    }
    printf("without final sum: %d\n", s_nlk);
    for(int i = 0; i < ADDED; i++){
        kmt->spin_lock(&usr_lk);
        s_lk++;
        kmt->spin_unlock(&usr_lk);
    }
    printf("with lock, final sum: %d \n", s_lk);
    while(1);
}
void test_sum(){
    for(int i = 0; i < NThread; i++){
        kmt->create(pmm->alloc(sizeof(task_t)), "Tsum", Tsum, NULL);
    }
}