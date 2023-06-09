#include <common.h>
#include <os.h>

sem_t fill, empty;
#define NThread 4
#define P kmt->sem_wait
#define V kmt->sem_signal

//static int s = 0;
#define NUM_PARE 10000
#define dep 5

/*
static void check(int turn, int sval){
  if(sval < 0 || sval > dep){
    printf("invalid turn%d, val: %d\n", turn, sval);
  }
}
static spinlock_t slk;
static void atomic_inc(){
  kmt->spin_lock(&slk);
  s++;
  kmt->spin_unlock(&slk);
}
static void atomic_dec(){
  kmt->spin_lock(&slk);
  s--;
  kmt->spin_unlock(&slk);
}
*/
void Tproduce(void * pc) {
  putstr("producer running\n");
  while (1) {
    P(&empty);
    putch('(');
    V(&fill);
  }
  putstr("producer finished\n");

  while(1) {
    assert(ienabled());
  }
}

void Tconsume(void * pc) {
  putstr("consumer running\n");
  while (1) {
    P(&fill);
    putch(')');
    V(&empty);
  }
  putstr("consumer finished\n");
  while(1) {
    assert(ienabled());
  }
}

void test_pc_sem(){
  printf("initiating pc-test\n");
  kmt->sem_init(&fill, "fill", 0);
  kmt->sem_init(&empty, "empty", dep);
  //kmt->spin_init(&slk, "slk");
  
  for(int i = 0; i < NThread; i++){
      kmt->create(tsk_alloc(), "producer", Tproduce, "abcdefgijkl" + i);
      kmt->create(tsk_alloc(), "consumer", Tconsume, "abcdefgh" + i);
  }
  printf("init pc-test finished\n");
}

