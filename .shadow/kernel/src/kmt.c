#include <os.h>

typedef struct Context_node{
    Context* ctx;
    struct Context_node * next;
} Context_node;
Context_node CONTEXTS_QUEUE_HEAD = { NULL, NULL}, * FRONT = &CONTEXTS_QUEUE_HEAD, *REAR = &CONTEXTS_QUEUE_HEAD;
Context_node* make_ctx_node(Context* ctx, Context_node* nxt){
    Context_node* new_nd = pmm->alloc(sizeof(Context_node));
    new_nd->ctx = ctx, new_nd->next = nxt;
    return new_nd;
}
static void PUSH(Context* ctx){
    REAR->next = make_ctx_node(ctx, REAR->next);
    REAR = REAR->next;
}
static Context* POP(){
    Context* popped = FRONT->next->ctx;
    Context_node* deleted = FRONT->next;
    FRONT->next = deleted->next;
    pmm->free(deleted);
    return popped;
}

#define TIMER_SEQ 1
Context* timer_intr_handler(Event ev, Context* ctx){
    PUSH(ctx);
    Context * next_ctx = POP();

    return next_ctx;
}

static void kmt_init(){
    os->on_irq(TIMER_SEQ, EVENT_IRQ_TIMER, timer_intr_handler);
}
// int  (*create)(task_t *task, const char *name, void (*entry)(void *arg), void *arg);
// void (*teardown)(task_t *task);
// void (*spin_init)(spinlock_t *lk, const char *name);
// void (*spin_lock)(spinlock_t *lk);
// void (*spin_unlock)(spinlock_t *lk);
// void (*sem_init)(sem_t *sem, const char *name, int value);
// void (*sem_wait)(sem_t *sem);
// void (*sem_signal)(sem_t *sem);

MODULE_DEF(kmt) = {
 .init = kmt_init
};


