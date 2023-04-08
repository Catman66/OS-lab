#include <kernel.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define ROUNDUP(a, sz)      ((((uintptr_t)a) + (sz) - 1) & ~((sz) - 1))
#define ROUNDDOWN(a, sz)    ((((uintptr_t)a)) & ~((sz) - 1))

void print_context(void* st, void* ed){
    for(uint8_t* p = st; p != ed; p++){
        printf("%x ", *p);
    }
}