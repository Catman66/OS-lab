#include "common.h"

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

spinlock_t lk = SPIN_INIT();

void LOCK(lock_t* lk){
    spin_lock(lk);
}

void UNLOCK(lock_t* lk){
    spin_unlock(lk);
}