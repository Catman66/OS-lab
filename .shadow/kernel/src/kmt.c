#include <os.h>

task_t* current[MAX_CPU], * tasks = NULL;   
spinlock_t task_lk;
Context * os_contexts[MAX_CPU];             //os idle thread context saved here
#define os_ctx (os_contexts[cpu_current()])

#define LOCK(lk) kmt->spin_lock((lk))
#define UNLOCK(lk) kmt->spin_unlock((lk))
int NTASK = 0;

void check_link_structure(){
    printf("checking tasks\n");
    kmt->spin_lock(&task_lk);
    
    task_t * p = tasks;
    for(int i = 0;i < NTASK; i++){
        panic_on(p == NULL, "error in check valid tasks: null ptr\n");
        p = p->next;
    }
    panic_on(p != tasks, "error in task-link structure, num error\n");    //a circle of exact number N
    
    kmt->spin_unlock(&task_lk);
    printf("check finished\n");
}

void print_tasks(){
    printf("=== current tasks: ");
    LOCK(&task_lk);
    task_t* p = tasks;
    for(int i = 0; i < NTASK; i++){
        printf("[%s, %d]", p->name, p->stat);
        p = p->next;
    }
    UNLOCK(&task_lk);
    printf("\n");
}

void save_context(Context* ctx){        //better not be interrupted
    bool i = ienabled();
    iset(false);
    if(curr == NULL){   //first save 
        os_ctx = ctx;   //always runnable
    } else {    
        curr->ctx = ctx;
        curr->stat = IN_INTR;
        panic_on(CANARY_ALIVE(curr) == false, "we lost the canary!!!\n");
    }
    iset(i);
}

Context * schedule(){
    if(tasks == NULL){      //no tasks
        return os_ctx;
    }
    LOCK(&task_lk);
    //print_tasks();
    if(curr == NULL){       //first useful schedule
        curr = tasks;
    }
    //printf("one switch\n");
    task_t * p = curr->next;
    //print_tasks();
    for(int n = 0; n < NTASK; n++){
        if(p->stat == RUNNABLE){
            curr = p;
            p->stat = RUNNING;
            UNLOCK(&task_lk);
            return p->ctx;
        }
        p = p->next;
    }
    UNLOCK(&task_lk);
    //no threads to be sched 
    printf("no thrads to sched\n");
    curr = NULL;
    return os_ctx;
}

#define TIMER_SEQ 1
Context* timer_intr_handler(Event ev, Context* ctx){
    if(curr != NULL){
        curr->stat = RUNNABLE;
    }
    return schedule();
}


/// page fault handler
Context* page_fault_handler(Event ev, Context* ctx){
    return ctx;             // return to the original program
}

spinlock_t usr_lk;
static void init_locks(){
    kmt->spin_init(&task_lk, "lock for task link");
    kmt->spin_init(&usr_lk, "user lock");
}
static void sign_irqs(){
    os->on_irq(TIMER_SEQ, EVENT_IRQ_TIMER, timer_intr_handler);
}
static void kmt_init(){
    printf("=== kmt init begin === \n");
    init_locks();
    printf("=== locks init finished ===\n");
    sign_irqs();
    printf("=== kmt init finished ===\n");
}

//need to mod global tasklist
static int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg){
    task->stack = pmm->alloc(STACK_SIZE);
    panic_on(task->stack == NULL, "fail to alloc stack \n");
    *(uint32_t*)task->stack = CANARY;       //in case stack overflow
    Area k_stk = (Area){ task->stack, task->stack + STACK_SIZE };
    task->ctx = kcontext(k_stk, entry, arg);
    task->stat = RUNNABLE;
    task->name = name;

    kmt->spin_lock(&task_lk);
    if(tasks == NULL){          //make a circle of one task
        tasks = task;
        tasks->next = tasks;
    } else {
        task->next = tasks->next;
        tasks->next = task;
    }
    NTASK++;
    kmt->spin_unlock(&task_lk);
    check_link_structure();
    return 0;
}
void kmt_teardown(task_t *task){
    //remove from link
    if(NTASK == 1){
        NTASK = 0;
        tasks = NULL;
    }
    task_t* pre = tasks, * p = tasks->next;
    for(int n = 0; n < NTASK; n++){
        if(p == task){
            pre->next = p->next;
            break;
        }
        pre = p, p = p->next;
    }

    pmm->free(task->stack);
}
#define HOLD 0
#define NHOLD 1
void kmt_spin_init(spinlock_t *lk, const char *name){
    lk->val = NHOLD;
}
void kmt_spin_lock(spinlock_t *lk){
    while(atomic_xchg(&(lk->val), NHOLD) == NHOLD){
        ;
    }
}
void kmt_spin_unlock(spinlock_t *lk){
    atomic_xchg(&(lk->val), HOLD);
}
// void (*sem_init)(sem_t *sem, const char *name, int value);
// void (*sem_wait)(sem_t *sem);
// void (*sem_signal)(sem_t *sem);

MODULE_DEF(kmt) = {
 .init = kmt_init,
 .create = kmt_create, 
 .teardown = kmt_teardown,
 .spin_init = kmt_spin_init,
 .spin_lock = kmt_spin_lock,
 .spin_unlock = kmt_spin_unlock
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
