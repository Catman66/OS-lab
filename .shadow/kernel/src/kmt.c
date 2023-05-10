#include <os.h>

task_t* current[MAX_CPU], * tasks = NULL;   
spinlock_t task_lk;

int NLOCK[MAX_CPU];
#define n_lk (NLOCK[cpu_current()])         //used by lock and unlock, 

Context *   os_contexts[MAX_CPU];             //os idle thread context saved here
#define os_ctx (os_contexts[cpu_current()])

#define LOCK(lk) kmt->spin_lock((lk))
#define UNLOCK(lk) kmt->spin_unlock((lk))
int NTASK = 0;

void check_task_link_structure();
void print_tasks();

void save_context(Context* ctx){        //better not be interrupted
    bool i = ienabled();
    iset(false);
    if(curr == NULL){   //first save 
        os_ctx = ctx;   //always runnable
    } else {    
        curr->ctx = ctx;
        panic_on(CANARY_ALIVE(curr) == false, "We lost the canary!!!\n");
    }
    iset(i);
}

Context * schedule(){
    if(tasks == NULL){      //no tasks
        return os_ctx;
    }
    LOCK(&task_lk);
    if(curr == NULL){       //first useful schedule
        curr = tasks;
    }
    task_t * p = curr->next;
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
    print_local("no threads to sched\n");
    curr = NULL;
    return os_ctx;
}

Context* timer_intr_handler(Event ev, Context* ctx){
    if(curr != NULL && curr->stat == RUNNING){
        curr->stat = RUNNABLE;
    }
    return schedule();
}
Context * yield_handler(Event ev, Context* ctx){
    return timer_intr_handler(ev, ctx);
}

/// page fault handler
Context* page_fault_handler(Event ev, Context* ctx){
    panic("page fault not implemented yet\n");
    return ctx;             // return to the original program
}

spinlock_t usr_lk;
static void init_locks(){
    for(int i = 0; i < MAX_CPU; i++){
        NLOCK[i] = 0;
    }
    kmt->spin_init(&task_lk, "lock for task link");
    kmt->spin_init(&usr_lk, "user lock");
}
static void sign_irqs(){
    os->on_irq(2, EVENT_IRQ_TIMER, timer_intr_handler);
    os->on_irq(1, EVENT_YIELD, yield_handler);
}
static void init_tasks(){
    for(int i = 0; i < MAX_CPU; i++){
        current[i] = NULL;
    }
}
static void kmt_init(){
    print_local("=== kmt init begin === \n");

    init_locks();       print_local("=== locks init finished ===\n");

    sign_irqs();        print_local("=== kmt init finished ===\n");

    init_tasks();     

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
    task->id = NTASK;
    if(tasks == NULL){          //make a circle of one task
        tasks = task;
        tasks->next = tasks;
    } else {
        task->next = tasks->next;
        tasks->next = task;
    }
    NTASK++;
    kmt->spin_unlock(&task_lk);
    check_task_link_structure();
    return 0;
}
static void kmt_teardown(task_t *task){
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
    lk->val = HOLD;
}
int PRE_INTR[MAX_CPU];
#define pre_i (PRE_INTR[cpu_current()])
void kmt_spin_lock(spinlock_t *lk){
    int i = ienabled();
    iset(false);
    pre_i = i;

    while (atomic_xchg(&(lk->val), NHOLD) == NHOLD) {
        //curr->stat = SLEEPING;
        //yield();            // fail to lock and sleep
        ;
    }
    __sync_synchronize();
    n_lk++;
}
void kmt_spin_unlock(spinlock_t *lk){
    n_lk--;

    __sync_synchronize();
    atomic_xchg(&(lk->val), HOLD);
    if(n_lk == 0){
        iset(pre_i);
    }
}

void kmt_sem_init(sem_t *sem, const char *name, int value){
    sem->desc = name;
    sem->val = value;
    kmt_spin_init(&(sem->lock), name);          
    sem->front = sem->rear = NULL;
}

P_task_node* make_new_semqueue_node(task_t* ctx, P_task_node* nxt){
    P_task_node * new_nd = pmm->alloc(sizeof(P_task_node));
    new_nd->p_task = ctx;
    new_nd->next = nxt;
    return new_nd;
}
void sem_enqueue_locked(sem_t* sem, task_t* tsk){
    if(SEM_NONE_WAITING(sem)){
        sem->front = sem->rear = make_new_semqueue_node(tsk, NULL);
    } else {
        sem->rear->next = make_new_semqueue_node(tsk, NULL);
        sem->rear = sem->rear->next;
    }
}
task_t* sem_rand_dequeue_locked(sem_t* sem){
    assert(SEM_NONE_WAITING(sem) == false);
    assert(sem->val < 0);
    task_t *        ret = sem->front->p_task;
    P_task_node *   del = sem->front;

    if(SEM_ONE_WAITING(sem)){
        sem->front = sem->rear = NULL;
    } else {
        sem->front = sem->front->next;
    }
    pmm->free(del);
    return ret;
}

void kmt_sem_wait(sem_t *sem){
    kmt_spin_lock(&sem->lock);
    sem->val --;
    if(sem->val < 0){
        curr->stat = SLEEPING;
        sem_enqueue_locked(sem, curr);
    } 
    kmt_spin_unlock(&sem->lock);
    if(curr->stat == SLEEPING){
        yield();
    }
}
void kmt_sem_signal(sem_t *sem){
    kmt_spin_lock(&sem->lock);
    if(sem->val < 0){
        sem_rand_dequeue_locked(sem)->stat = RUNNABLE;
    }
    sem->val++;
    kmt_spin_unlock(&sem->lock);
}
MODULE_DEF(kmt) = {
 .init = kmt_init,
 .create = kmt_create, 
 .teardown = kmt_teardown,
 .spin_init = kmt_spin_init,
 .spin_lock = kmt_spin_lock,
 .spin_unlock = kmt_spin_unlock,
 .sem_init = kmt_sem_init,
 .sem_signal = kmt_sem_signal,
 .sem_wait = kmt_sem_wait
};

void check_task_link_structure(){
    print_local("checking tasks\n");
    kmt->spin_lock(&task_lk);
    
    task_t * p = tasks;
    for(int i = 0;i < NTASK; i++){
        panic_on(p == NULL, "error in check valid tasks: null ptr\n");
        p = p->next;
    }
    panic_on(p != tasks, "error in task-link structure, num error\n");    //a circle of exact number N
    
    kmt->spin_unlock(&task_lk);
    print_local("check finished\n");
}

void print_tasks(){
    print_local("=== current tasks: ");
    LOCK(&task_lk);
    task_t* p = tasks;
    for(int i = 0; i < NTASK; i++){
        print_local("[%s, %d]", p->name, p->stat);
        p = p->next;
    }
    UNLOCK(&task_lk);
    print_local("\n");
}
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
