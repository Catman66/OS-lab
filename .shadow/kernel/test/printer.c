#include <common.h>

void print_context(void* st, void* ed){
    printf("===Contex:");
    for(uint8_t* p = st; p != ed; p++){
        printf("%x ", *p);
    }
    printf("===\n");
}