#include <os.h>

task_t* current[MAX_CPU], * tasks = NULL;   
Context * os_contexts[MAX_CPU];             //os idle thread context saved here
#define os_ctx (os_contexts[cpu_current()])

int NTASK = 0;

bool check_tasks(){
    printf("checking tasks\n");

    task_t * p = tasks;
    for(int i = 0;i < NTASK; i++){
        panic_on(p == NULL, "error in check valid tasks: null ptr in\n");
        p = p->next;
    }

    printf("check finished\n");
    return p == curr;
}

void save_context(Context* ctx){
    if(curr == NULL){   //first save 
        os_ctx = ctx;   //always runnable
    } else {
        curr->ctx = ctx;
        curr->stat = IN_INTR;
    }
}

Context * sched(){
    task_t * p = curr->next;
    for(int n = 0; n < NTASK; n++){
        if(p->stat == RUNNABLE){
            curr = p;
            return p->ctx;
        }
    }
    //no threads to be sched 
    printf("no thrads to sched\n");
    return os_ctx;
}

#define TIMER_SEQ 1
Context* timer_intr_handler(Event ev, Context* ctx){
    curr->stat = RUNNABLE;
    return sched();
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
    panic_on(task->stack == NULL, "fail to alloc stack \n");
    Area k_stk = (Area){ task->stack, task->stack + STACK_SIZE };
    task->ctx = kcontext(k_stk, entry, arg);
    task->stat = RUNNABLE;

    //current,当前处理器
    if(tasks == NULL){          //make a circle of one task
        tasks = task;
        tasks->next = tasks;
    } else {
        task->next = tasks->next;
        tasks->next = task;
    }
    NTASK++;
    assert(check_tasks());
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
