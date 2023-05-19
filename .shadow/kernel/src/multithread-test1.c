#include <common.h>
#include <os.h>

int cnt = 0;
void Tyield(void * arg){
    while(1){
        assert(ienabled());
        printf("%d ", cnt++);
        yield();
    }
}

void yield_test(){
    putstr("This is yield test\n");
    for(int i = 0; i < 8; i++){
        kmt->create(tsk_alloc(), "Yield-thread", Tyield, &"abcdefgh"[i]);
    }
    putstr("yield test initiated\n");
}

void thread_switch_test(){
    putstr("One thread, switching between CPU\n");
    kmt->create(tsk_alloc(), "single-thread", Tyield, "a");
}