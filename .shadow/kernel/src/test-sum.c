#include <os.h>
#include<common.h>

#define NThread 4
#define ADDED 100000
static volatile int s_nlk = 0, s_lk = 0;
static volatile int tool = 1; 
extern spinlock_t usr_lk;
int handle_val(volatile int v){
    for(int i = 0; i < 100; i ++){
        tool += v;
    }
    return v + 1;
}

void Tsum(){
    for(int i = 0; i < ADDED; i++){
        int tmpt = s_nlk;
        tmpt = handle_val(tmpt);
        s_nlk = tmpt;
    }
    printf("without lock, final sum: %d\n", s_nlk);
    for(int i = 0; i < ADDED; i++){
        kmt->spin_lock(&usr_lk);
        int tmpt = s_lk;
        tmpt = handle_val(tmpt);
        s_lk = tmpt;
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