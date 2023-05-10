#include <common.h>
#include <os.h>

sem_t fill, empty;
#define NThread 1
#define P kmt->sem_wait
#define V kmt->sem_signal

#define NUM_PARE 50
void Tproduce(void * pc) {
  int i = 0;
  while (i++ < NUM_PARE) {
    P(&empty);
    printf("(%c", *(char*)pc);
    V(&fill);
  }
  printf("finished \n");
  while(1) {
    //printf("after %c finish\n", *(char *)pc);
  }
}

void Tconsume(void * pc) {
  int i = 0;
  while (i++ < NUM_PARE) {
    P(&fill);
    printf(")%c", *(char*)pc);
    V(&empty);
  }
  printf("finished\n");
  while(1) {
    //printf("after %c finish\n", *(char*)pc);
  }
}

void test_pc_sem(){
  printf("initiating pc-test\n");
  kmt->sem_init(&fill, "fill", 0);
  kmt->sem_init(&empty, "empty", 2);
  for(int i = 0; i < NThread; i++){
      kmt->create(pmm->alloc(sizeof(task_t)), "producer", Tproduce, "abcdefgijkl" + i);
      kmt->create(pmm->alloc(sizeof(task_t)), "consumer", Tconsume, "abcdefgh" + i);
  }
  printf("init pc-test finished\n");
}

