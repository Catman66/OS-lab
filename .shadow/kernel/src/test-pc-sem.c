#include <common.h>
#include <os.h>

sem_t fill, empty;
#define NThread 4
#define P kmt->sem_wait
#define V kmt->sem_signal

static int s = 0;
#define NUM_PARE 10000
#define dep 5

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

void Tproduce(void * pc) {
  int i = 0;
  putstr("producer running\n");
  while (i++ < NUM_PARE) {
    P(&empty);
    //printf("(%c", *(char*)pc);
    atomic_inc();
    V(&fill);
    check(i, s);
  }
  putstr("producer finished\n");

  while(1) {
    assert(ienabled());
  }
}

void Tconsume(void * pc) {
  int i = 0;
  putstr("consumer running\n");
  while (i++ < NUM_PARE) {
    P(&fill);
    //printf(")%c", *(char*)pc);
    atomic_dec();
    V(&empty);
    check(i, s);
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
  
  for(int i = 0; i < NThread; i++){
      kmt->create(tsk_alloc(), "producer", Tproduce, "abcdefgijkl" + i);
      kmt->create(tsk_alloc(), "consumer", Tconsume, "abcdefgh" + i);
  }
  printf("init pc-test finished\n");
}

