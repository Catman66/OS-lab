#include <common.h>
#include <os.h>

void Tyield(void * arg){
    while(1){
        assert(ienabled());
        yield();
    }
}

void yield_test(){
    putstr("This is yield test\n");
    for(int i = 0; i < 8; i++){
        kmt->create(tsk_alloc(), "Yield-thread", Tyield, NULL);
    }
    putstr("yield test initiated\n");
}
