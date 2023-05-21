#include <os.h>

#define LOCK kmt->spin_lock
#define UNLOCK kmt->spin_unlock

#define MAX_NTASK 32
#define NTASK_PERCPU 16
task_t * current[MAX_CPU], * task_pool[MAX_NTASK], *last_tsk[MAX_CPU];

spinlock_t ntask_lk;

int     NLOCK[MAX_CPU];
#define n_lk (NLOCK[cpu_current()])         //used by lock and unlock, 

Context * os_contexts[MAX_CPU];             //os idle thread context saved here
#define os_ctx (os_contexts[cpu_current()])

int NTASK = 0;
int last_sched = 0;

void simple_lock(int * lk){
    while(atomic_xchg(lk, 1)){
        ;
    }
}
void simple_unlock(int *lk){
    atomic_xchg(lk, 0);
}

//init components

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

void enable_last_task(int c){
    task_t * last = last_tsk[c];
    if(last != NULL && last != curr){           
        simple_lock(&last->lock);

        assert(last->cpu == c);
        last->cpu = -1;
        
        simple_unlock(&last->lock);
    }
}

void save_context(Context* ctx){        //better not be interrupted
    assert(ienabled() == false);

    if(curr == NULL){   //save from os-thread
        os_ctx = ctx;   //always runnable
    } else {       
        simple_lock(&curr->lock);         
        assert(curr->cpu == cpu_current());
        assert(curr->canary1 == CANARY && curr->canary2 == CANARY);

        curr->ctx = ctx;

        simple_unlock(&curr->lock);
    }
}

Context * schedule(){
    assert(ienabled() == false);
    if(NTASK == 0){
        return os_ctx;
    }

    task_t * tsk;                        //try to swap from another cpu
    
    int i = (last_sched + 1) % NTASK;

    for(int cnt = 0; cnt < NTASK; i = (i + 1) % NTASK, cnt++){
        tsk = task_pool[i];

        simple_lock(&tsk->lock);
        if(tsk->cpu == -1){
            tsk->cpu = cpu_current();
            last_sched = i;
            simple_unlock(&tsk->lock);

            last_tsk[cpu_current()] = curr;
            curr = tsk;

            return tsk->ctx;

        } else {
            simple_unlock(&tsk->lock);
        }
    }
    
    //no task avaliable
    last_tsk[cpu_current()] = curr;
    curr = NULL;

    return os_ctx;
}

Context* timer_intr_handler(Event ev, Context* ctx){
    assert(ienabled() == false);
    return NULL;
}

//need to mod global tasklist
static int kmt_create(task_t *tsk, const char *name, void (*entry)(void *arg), void *arg){
    panic_on(NTASK + 1 > MAX_NTASK, "too much tasks\n");
    panic_on(tsk == NULL, "fail to alloc task \n");
    
    Area k_stk = (Area){ tsk->stack, (void*)tsk->stack + OS_STACK_SIZE };
    tsk->ctx = kcontext(k_stk, entry, arg);

    tsk->name = name;
    tsk->lock = 0;
    tsk->cpu = -1;
    tsk->canary1 = tsk->canary2 = CANARY;

    kmt->spin_lock(&ntask_lk);
    simple_lock(&tsk->lock);

    task_pool[NTASK++] = tsk;

    simple_unlock(&tsk->lock);
    kmt->spin_unlock(&ntask_lk);

    panic_on(cross_check(tsk) == false, "tasks stack cross!!!\n");
    return 0;
}

static void kmt_teardown(task_t *task){
    panic_on(NTASK < 0, "no task to teardown\n");
    kmt->spin_lock(&ntask_lk);
    for(int i = 0; i < NTASK; i++){
        if(task_pool[i] == task){
            task_pool[i] = task_pool[NTASK - 1];

            pmm->free(task->stack);
            NTASK--;

            kmt->spin_unlock(&ntask_lk);
            return;
        }
    }
    panic("fail to find wanted task to teardown\n");
}

//lock 

int PRE_INTR[MAX_CPU];
#define pre_i   (PRE_INTR[cpu_current()])
#define HOLD    0
#define NHOLD   1

void kmt_spin_init(spinlock_t *lk, const char *name){
    lk->desc = name;
    lk->val = HOLD;
}

void kmt_spin_lock(spinlock_t *lk){
    int i = ienabled();
    iset(false);
    if(n_lk == 0){
        pre_i = i;
    }
    while (atomic_xchg(&(lk->val), NHOLD) == NHOLD) {
        ;
    }
    n_lk++;
    panic_on(ienabled(), "i set in lock\n");
}

void kmt_spin_unlock(spinlock_t *lk){
    panic_on(n_lk < 1, curr->name);
    assert(lk->val == NHOLD);
    assert(ienabled() == false);

    n_lk--;

    atomic_xchg(&(lk->val), HOLD);
    if(n_lk == 0){                  //intr protects n_lk
        iset(pre_i);
    }
}

//semaphore 

void kmt_sem_init(sem_t *sem, const char *name, int value){
    sem->name   = name;
    sem->lock   = 0;
    sem->val    = value;
}

void kmt_sem_wait(sem_t *sem){
    //assert(ienabled());           tty can P when with intr off
    
    while(1){
        simple_lock(&sem->lock);
        if(sem->val > 0){
            sem->val --;
            simple_unlock(&sem->lock);
            break;
        }
        simple_unlock(&sem->lock);
        yield();
    }
}

void kmt_sem_signal(sem_t *sem){
    simple_lock(&sem->lock);
    sem->val++;
    simple_unlock(&sem->lock);
}

MODULE_DEF(kmt) = {
 .init          = kmt_init,
 .create        = kmt_create, 
 .teardown      = kmt_teardown,
 .spin_init     = kmt_spin_init,
 .spin_lock     = kmt_spin_lock,
 .spin_unlock   = kmt_spin_unlock,
 .sem_init      = kmt_sem_init,
 .sem_signal    = kmt_sem_signal,
 .sem_wait      = kmt_sem_wait
};

static void init_locks(){
    for(int i = 0; i < MAX_CPU; i++){
        NLOCK[i] = 0;
    }
    kmt->spin_init(&ntask_lk, "lock for task link");
}

static void sign_irqs(){
    os->on_irq(2, EVENT_IRQ_TIMER, timer_intr_handler);
    os->on_irq(1, EVENT_YIELD, timer_intr_handler);
    os->on_irq(1000, EVENT_NULL, schedule);
}

static void init_tasks(){
    for(int i = 0; i < MAX_CPU; i++){
        current[i] = last_tsk[i] = NULL;
    }

    for(int i =0; i < MAX_NTASK; i++){
        task_pool[i] = NULL;
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

bool cross_check(task_t* tsk){
    for(int i = 0; i < NTASK; i++){
        if(task_pool[i] != tsk){
            task_t * another = task_pool[i];
            if(another + 1 < tsk || tsk + 1 < another){
                continue;
            }
            return false;
        }
    }
    return true;
}


