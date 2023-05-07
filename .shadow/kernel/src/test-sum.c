#include <os.h>
#include<common.h>

#define NThread 4
#define ADDED 100000
static volatile int s_nlk = 0, s_lk = 0;
static volatile int tool = 1; 
extern spinlock_t usr_lk;
int handle_val(int v){
    for(int i = 0; i < 100; i ++){
        tool += v;
    }
    return v + 1;
}

void Tsum(void* name){
    for(int i = 0; i < ADDED; i++){
        s_nlk++;
    }
    printf("[%s]: without lock, final sum: %d, expected: %d\n",(const char*)name, s_nlk, NThread * ADDED);
    for(int i = 0; i < ADDED; i++){
        kmt->spin_lock(&usr_lk);
        s_lk++;
        kmt->spin_unlock(&usr_lk);
    }
    kmt->spin_lock(&usr_lk);
    printf("with lock, final sum: %d \n", s_lk);
    kmt->spin_unlock(&usr_lk);
    
    while(1);       //never return 
}

const char * thread_names[NThread] = {
    "T1", "T2", "T3", "T4"
};

void test_sum(){
    for(int i = 0; i < NThread; i++){
        kmt->create(pmm->alloc(sizeof(task_t)), thread_names[i], Tsum, (void*)thread_names[i]);
    }
}