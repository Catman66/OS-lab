#include <common.h>

#define CANARY 0xa5a55a5a
#define STACK_SIZE 8192
typedef enum{ RUNNING, RUNNABLE, IN_INTR} task_stat;

struct task {
  Context * ctx;
  task_stat stat;
  struct task * next;
  int canary;
  uint8_t * stack;
};

#define MAX_CPU 8
extern task_t* current[MAX_CPU];
#define curr (current[cpu_current()])

void save_context(Context* ctx);

struct spinlock {
  // TODO
};

struct semaphore {
  // TODO
};
