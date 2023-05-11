#include <common.h>
#include <os.h>

sem_t fill, empty;
#define NThread 4
#define P kmt->sem_wait
#define V kmt->sem_signal

static int s = 0;
#define NUM_PARE 1000
#define dep 3

static void check(int turn, int sval){
  if(sval < 0 || sval > dep){
    printf("invalid turn%d, val: %d\n", turn, sval);
  }
}

void Tproduce(void * pc) {
  char c = *(char*)pc;
  int i = 0;
  while (i++ < NUM_PARE) {
    P(&empty);
    //printf("(%c", *(char*)pc);
    s++;
    V(&fill);
    check(i, s);
  }
  printf("producer %c finished \n", c);
  while(1) {
    //printf("after %c finish\n", *(char *)pc);
  }
}

void Tconsume(void * pc) {
  char c = *(char*)pc;
  int i = 0;
  while (i++ < NUM_PARE) {
    P(&fill);
    //printf(")%c", *(char*)pc);
    s--;
    V(&empty);
    check(i, s);
  }
  printf("consumer %c finished\n", c);
  while(1) {
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

