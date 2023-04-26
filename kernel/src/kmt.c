#include <os.h>

task_t* current = NULL;

#define TIMER_SEQ 1
Context* timer_intr_handler(Event ev, Context* ctx){
    current->ctx = ctx;
    current = current->next;
    return current->ctx;
}

/// page fault handler
Context* page_fault_handler(Event ev, Context* ctx){
    return ctx;             // return to the original program
}

static void kmt_init(){
    os->on_irq(TIMER_SEQ, EVENT_IRQ_TIMER, timer_intr_handler);
    printf("kmt init finished\n");
}

static int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg){
    //创建一个新的线程,首个线程不会有这个过程，有点棘手，current只能在中断调用的时候来
    task->canary = CANARY;
    task->stack = pmm->alloc(STACK_SIZE);
    Area k_stk = (Area){ task->stack, task->stack + STACK_SIZE };
    task->ctx = kcontext(k_stk, entry, arg);
    
    ucontext(,)
    //current 
    assert(current != NULL);
    task->next = current->next;
    current->next = task;
    return 0;
}
// void (*teardown)(task_t *task);
// void (*spin_init)(spinlock_t *lk, const char *name);
// void (*spin_lock)(spinlock_t *lk);
// void (*spin_unlock)(spinlock_t *lk);
// void (*sem_init)(sem_t *sem, const char *name, int value);
// void (*sem_wait)(sem_t *sem);
// void (*sem_signal)(sem_t *sem);

MODULE_DEF(kmt) = {
 .init = kmt_init,
 .create = kmt_create
};


/*
static void PUSH(* ctx){
    REAR->next = make_ctx_node(ctx, REAR->next);
    REAR = REAR->next;
}
static Context* POP(){
    Context* popped = FRONT->next->ctx;
    Task_node* deleted = FRONT->next;
    FRONT->next = deleted->next;
    pmm->free(deleted);
    return popped;
}

*/
