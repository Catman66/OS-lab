#include"common.h"

#define SCALE           1000000
#define MAX_ALLOC_SZ    1024
#define STK_SZ          10000
#define NUM_REPORT      100000
typedef struct{
    int top;
    void* content[STK_SZ];
} STK;

bool    empty(STK* stk);
bool    full(STK* stk);
void    rand_push(STK* stk, void* ptr);
void*   pop();

void random_test();
void threads_test();

void do_alloc(STK* stk);
void do_free(STK* stk);
typedef enum{ ACT_ALLOC = 0, ACT_FREE = 1 } ACT; 

ACT     rand_act();
size_t  rand_alloc_sz();

