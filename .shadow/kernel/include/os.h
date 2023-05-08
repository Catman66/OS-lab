#include <common.h>

#define CANARY 0xa5a55a5a
#define CANARY_ALIVE(t) (*(uint32_t*)((t)->stack) == CANARY)
#define STACK_SIZE 8192
typedef enum{ RUNNING, RUNNABLE, IN_INTR, SLEEPING } task_stat;

struct task {
  const char * name;
  Context * ctx;
  task_stat stat;
  struct task * next;
  uint8_t * stack;   
  int num_lock;
};

#define MAX_CPU 8
extern task_t* current[MAX_CPU];
#define curr (current[cpu_current()])

void save_context(Context* ctx);

struct spinlock {
  const char * desc;
  int val;
};

typedef struct P_task_node {
  task_t* p_task;
  struct P_task_node * next;
} P_task_node;
#define SEM_EMPTY(head) ((head).next == NULL)
struct semaphore {
  const char * desc;
  int val;
  spinlock_t lock;
  P_task_node queue;
};

