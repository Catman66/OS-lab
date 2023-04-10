#include "common.h"

static inline int atomic_xchg(volatile int *addr, int newval) {
  int result;
  asm volatile ("lock xchg %0, %1":
    "+m"(*addr), "=a"(result) : "1"(newval) : "memory");
  return result;
}

typedef int spinlock_t;
#define SPIN_INIT() 0
void spin_lock(spinlock_t *lk) {
  while (1) {
    int value = atomic_xchg(lk, 1);
    if (value == 0) {
      break;
    }
  }
}
void spin_unlock(spinlock_t *lk) {
  atomic_xchg(lk, 0);
}

void LOCK(lock_t* lk){
    spin_lock(lk);
}

void UNLOCK(lock_t* lk){
    spin_unlock(lk);
}