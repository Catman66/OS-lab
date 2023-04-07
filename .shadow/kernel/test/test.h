#include"common.h"

#define SCALE 10


#define alloc_sz 1024
struct alloc_tst{
    int sz;
    void* ptr;
} cases[SCALE] = {
    {}
};
typedef struct alloc_tst alloc_tst;

#define END_SP(c) ((c).ptr + (c).sz)
#define START_SP(c) ((c).ptr)
#define ALLOC_FAIL(c) ((c).ptr == NULL)
void do_all_alloc();
void check_overlap();
void check_in_heap();
void check_align();

void check();



#define EMPTY()         (top == -1)
#define FULL()          (top == STK_SZ - 1)
#define PUSH(ptr)       (STK[++top] = (ptr))
#define POP()           (STK[top--])
#define INIT_STK()      (top = -1)

void push_rand(void* ptr){
    int idx = rand() % (top + 1);
    STK[++top] = STK[idx];
    STK[idx] = ptr;
}

void random_test();


