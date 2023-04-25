#include <common.h>

#define CANARY 0xa5a55a5a
#define STACK_SIZE 8192
struct task {
  Context * ctx;
  struct task * next;
  int canary;
  uint8_t * stack;
};

struct spinlock {
  // TODO
};

struct semaphore {
  // TODO
};
