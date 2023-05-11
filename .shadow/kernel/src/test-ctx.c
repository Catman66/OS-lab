#include <common.h>
#include <os.h>


static void Ttest(void * arg){
    uint8_t taken[176];
    for(int i = 0 ; i < 176; i++){
        printf("[%d]", taken[i]);
        yield();
    }
    while(1);
}

const char * names[] = {
    "T0","T1","T2","T3" 
};

void test_ctx(){
    for(int i = 0; i < 1; i++){
        kmt->create(tsk_alloc(), names[i], Ttest, NULL);
    }
}