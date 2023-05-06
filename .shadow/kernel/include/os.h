#include <common.h>

#define CANARY 0xa5a55a5a
#define CANARY_ALIVE(t) (*(uint32_t*)((t)->stack) == CANARY)
#define STACK_SIZE 8192
typedef enum{ RUNNING = 0, RUNNABLE = 1, IN_INTR = 2} task_stat;

struct task {
  const char * name;
  Context * ctx;
  task_stat stat;
  struct task * next;
  uint8_t * stack;    
};

#define MAX_CPU 8
extern task_t* current[MAX_CPU];
#define curr (current[cpu_current()])

void save_context(Context* ctx);

struct spinlock {
  int val;
};

struct semaphore {
  // TODO
};
