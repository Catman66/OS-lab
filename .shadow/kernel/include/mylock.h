#include<am.h>

#ifndef _MY_LOCK_H_
#define _MY_LOCK_H_

typedef int lock_t;
#define SPIN_INIT() 0
void PMM_LOCK(lock_t *lk) {
  while (1) {
    int value = atomic_xchg(lk, 1);
    if (value == 0) {
      break;
    }
  }
}
void PMM_UNLOCK(lock_t *lk) {
  atomic_xchg(lk, 0);
}

#endif