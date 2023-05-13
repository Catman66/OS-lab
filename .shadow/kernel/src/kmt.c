#include <os.h>

#define MAX_NTASK 32
task_t * current[MAX_CPU], * task_pool[MAX_NTASK];
int last_sched = 0;

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
void dump_task_info(task_t* tsk);
void print_tasks();
void save_context(Context* ctx){        //better not be interrupted
    assert(ienabled() == false);
    n_switch++;

    if(curr == NULL){   //save from os-thread
        os_ctx = ctx;   //always runnable
    } else {       
        kmt->spin_lock(&curr->lock);         
        assert(curr->cpu == cpu_current());
        
        curr->ctx = ctx;
        curr->cpu = -1;
        
        assert(sane_task(curr));
        kmt->spin_unlock(&curr->lock);
    }
}

Context * schedule(){
    assert(ienabled() == false);
    if(NTASK == 0){
        return os_ctx;
    }

    int i = (last_sched + 1) % NTASK;
    task_t* p;
    for(int cnt = 0; cnt < NTASK; i = (i+ 1) % NTASK, cnt++){
        p = task_pool[i];
        kmt->spin_lock(&p->lock);
        if(p->stat == RUNNABLE){
            assert(p->cpu == -1);

            p->stat = RUNNING;
            kmt->spin_unlock(&p->lock);
            curr = p;
            last_sched = i;
            return curr->ctx;
        } else {
            kmt->spin_unlock(&p->lock);
        }
    }
    curr = NULL;
    print_local("no threads to sched\n");
    return os_ctx;
}

Context* timer_intr_handler(Event ev, Context* ctx){
    assert(ienabled() == false);
    if(curr != NULL){
        kmt->spin_lock(&curr->lock);
        curr->stat = RUNNABLE;
        kmt->spin_unlock(&curr->lock);
    }
    return schedule();
}

Context * yield_handler(Event ev, Context* ctx){
    return timer_intr_handler(ev, ctx);
}

//need to mod global tasklist
static int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg){
    panic_on(NTASK + 1 > MAX_NTASK, "too much tasks\n");
    panic_on(task == NULL, "fail to alloc task \n");
    
    Area k_stk = (Area){ task->stack + sizeof(task_t), (void*)task->stack + OS_STACK_SIZE };
    task->ctx = kcontext(k_stk, entry, arg);
    task->stat = RUNNABLE;
    task->name = name;
    task->canary1 = task->canary2 = CANARY;
    kmt->spin_init(&task->lock, name);
    task->cpu = -1;

    kmt->spin_lock(&task_lk);
    kmt->spin_lock(&task->lock);

    task->id = NTASK; 
    task_pool[NTASK++] = task;

    kmt->spin_unlock(&task->lock);
    kmt->spin_unlock(&task_lk);
    return 0;
}

static void kmt_teardown(task_t *task){
    panic_on(NTASK < 0, "no task to teardown\n");
    kmt->spin_lock(&task_lk);
    for(int i = 0; i < NTASK; i++){
        if(task_pool[i] == task){
            task_pool[i] = task_pool[NTASK - 1];

            pmm->free(task->stack);
            NTASK--;

            kmt->spin_unlock(&task_lk);
            return;
        }
    }
    panic("fail to find wanted task to teardown\n");
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
    sem->tp = -1;
    sem->cnt = 0;                       //P or V operation adds cnt
    sem->using = false;
}

P_task_node* make_new_semqueue_node(task_t* tsk, P_task_node* nxt){
    P_task_node * new_nd = pmm->alloc(sizeof(P_task_node));
    new_nd->p_task = tsk;
    new_nd->next = nxt;
    return new_nd;
}

void sem_enqueue_locked(sem_t* sem, task_t* tsk){
    sem->waiting_tsk[++sem->tp] = tsk;
    assert(sem->tp < SEM_WAITING_LEN);
}

task_t* sem_dequeue_locked(sem_t* sem){
    assert(SEM_NONE_WAITING(sem) == false);
    return sem->waiting_tsk[sem->tp--];
}


void kmt_sem_wait(sem_t *sem){
    assert(ienabled());
    kmt_spin_lock(&sem->lock);
    assert(sem->using == false);
    sem->using = true;

    assert(ienabled() == false);
    sem->val --;
    int blc = sem->val < 0;
    if(blc){
        if(curr != NULL && curr->stat != RUNNING) {panic("should be running\n");}
        curr->stat = SLEEPING;
        sem_enqueue_locked(sem, curr);
    } 
    
    sem->using = false;
    kmt_spin_unlock(&sem->lock);
    if(blc){
        yield();
    } 
}
void kmt_sem_signal(sem_t *sem){
    kmt_spin_lock(&sem->lock);
    assert(sem->using == false);
    sem->using = true;
    if(sem->val < 0){
        task_t* wakend = sem_dequeue_locked(sem);
        assert(wakend->stat == SLEEPING);
        wakend->stat = RUNNABLE;
    }
    sem->val++;
    sem->using = false;
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
    for(int i =0; i < MAX_NTASK; i++){
        task_pool[i] = NULL;
    }
    last_sched = 0;
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

void dump_task_info(task_t * tsk){
    printf("task_info: id: %d, rip: %p, rsp %p\n", tsk->id, X86_64_CTX(tsk->ctx)->rip, X86_64_CTX(tsk->ctx)->rsp); 
}