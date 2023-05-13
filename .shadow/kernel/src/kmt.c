#include <os.h>

task_t * current[MAX_CPU], * tasks = NULL;   
spinlock_t task_lk;

int     NLOCK[MAX_CPU];
#define n_lk (NLOCK[cpu_current()])         //used by lock and unlock, 

int     N_SWITCH[MAX_CPU];
#define n_switch (N_SWITCH[cpu_current()])

Context *   os_contexts[MAX_CPU];             //os idle thread context saved here
#define os_ctx (os_contexts[cpu_current()])

int NTASK = 0;

static void init_locks();

static void sign_irqs();

static void init_tasks();

static void kmt_init(){
    print_local("=== kmt init begin === \n");

    init_locks();       print_local(" === locks init finished ===\n");

    sign_irqs();        print_local(" === kmt init finished ===\n");

    init_tasks(); 

    print_local("=== num of tasks current: %d ===\n", NTASK);    
}

void check_task_link_structure();
void print_tasks();
void print_error_task();
void save_context(Context* ctx){        //better not be interrupted
    assert(ienabled() == false);
    n_switch++;
    if(curr == NULL){   //first save 
        os_ctx = ctx;   //always runnable
    } else {    
        curr->ctx = ctx;
        if(sane_task(curr) == false) {
            print_error_task(curr);
            assert(0);
        }
    }
}

Context * schedule(){
    assert(ienabled() == false);
    if(tasks == NULL){      //no tasks
        return os_ctx;
    }
    kmt->spin_lock(&task_lk);
    
    if(curr == NULL){       //first useful schedule
        curr = tasks;
    }
    task_t * p = curr->next;
    for(int n = 0; n < NTASK; n++){
        if(p->stat == RUNNABLE){
            curr = p;
            p->stat = RUNNING;
            kmt->spin_unlock(&task_lk);
            return p->ctx;
        }
        p = p->next;
    }
    kmt->spin_unlock(&task_lk);
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

//need to mod global tasklist
static int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg){
    panic_on(task->stack == NULL, "fail to alloc stack \n");
    Area k_stk = (Area){ task->stack, (void*)task->stack + OS_STACK_SIZE };
    task->ctx = kcontext(k_stk, entry, arg);
    task->stat = RUNNABLE;
    task->name = name;
    task->canary1 = task->canary2 = CANARY;
    
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
    lk->desc = name;
    lk->val = HOLD;
}
int PRE_INTR[MAX_CPU];
#define pre_i (PRE_INTR[cpu_current()])
void kmt_spin_lock(spinlock_t *lk){
    int i = ienabled();
    iset(false);
    if(n_lk == 0){
        pre_i = i;
    }
    while (atomic_xchg(&(lk->val), NHOLD) == NHOLD) {
        //curr->stat = SLEEPING;
        //yield();            // fail to lock and sleep
        ;
    }
    __sync_synchronize();
    n_lk++;
    panic_on(ienabled(), "i set in lock\n");
}

void kmt_spin_unlock(spinlock_t *lk){
    panic_on(n_lk < 1, curr->name);
    assert(lk->val == NHOLD);
    assert(ienabled() == false);

    n_lk--;
    __sync_synchronize();

    atomic_xchg(&(lk->val), HOLD);
    if(n_lk == 0){                  //intr protects n_lk
        iset(pre_i);
    }
}

void kmt_sem_init(sem_t *sem, const char *name, int value){
    sem->desc = name;
    sem->val = value;
    kmt_spin_init(&(sem->lock), name);          
    sem->front = sem->rear = NULL;
    sem->cnt = 0;                       //P or V operation adds cnt
}

P_task_node* make_new_semqueue_node(task_t* tsk, P_task_node* nxt){
    P_task_node * new_nd = pmm->alloc(sizeof(P_task_node));
    new_nd->p_task = tsk;
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
task_t* sem_dequeue_locked(sem_t* sem){
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
    assert(ienabled());
    kmt_spin_lock(&sem->lock);
    sem->val --;
    int blc = sem->val < 0;
    if(blc){
        if(curr->stat != RUNNING) { printf("should be running, but: %d\n", curr->stat); }
        curr->stat = SLEEPING;
        sem_enqueue_locked(sem, curr);
    } 
    kmt_spin_unlock(&sem->lock);
    if(blc){
        yield();
    } 
}
void kmt_sem_signal(sem_t *sem){
    kmt_spin_lock(&sem->lock);
    if(sem->val < 0){
        task_t* wakend = sem_dequeue_locked(sem);
        assert(wakend->stat == SLEEPING);
        wakend->stat = RUNNABLE;
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

/// page fault handler
Context* page_fault_handler(Event ev, Context* ctx){
    panic("page fault not implemented yet\n");
    return ctx;             // return to the original program
}

static void init_locks(){
    for(int i = 0; i < MAX_CPU; i++){
        NLOCK[i] = 0;
    }
    kmt->spin_init(&task_lk, "lock for task link");
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

struct X86_64_Context {
  void    *cr3;
  uint64_t rax, rbx, rcx, rdx,
           rbp, rsi, rdi,
           r8, r9, r10, r11,
           r12, r13, r14, r15,
           rip, cs, rflags,
           rsp, ss, rsp0;
};
#define TXT_END 0x110000
#define X86_64_CTX(ctx) ((struct X86_64_Context * )(ctx))

bool sane_task(task_t * tsk){
    struct X86_64_Context * ctx = X86_64_CTX(tsk->ctx);
    return ctx->rip < TXT_END 
    && 
    ctx->rsp > (intptr_t)(&(tsk->canary2)) && ctx->rsp <= (uintptr_t)(tsk->stack) + OS_STACK_SIZE
    && 
    tsk->canary1 == CANARY && tsk->canary2 == CANARY;
}

void print_error_task(task_t * tsk){
    printf("%dth switch,ctx@%p, id: %d, rip:%lx, rsp: %lx\n", 
    n_switch, tsk->ctx, tsk->id, X86_64_CTX(tsk->ctx)->rip, X86_64_CTX(tsk->ctx)->rsp);
}