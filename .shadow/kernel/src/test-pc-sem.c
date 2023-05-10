#include <common.h>
#include <os.h>

sem_t fill, empty;
#define NThread 4
#define P kmt->sem_wait
#define V kmt->sem_signal

#define NUM_PARE 5000
void Tproduce(void * p_c) {
  int i = 0;
  while (i++ < NUM_PARE) {
    P(&empty);
    printf("(%c", *(char*)p_c);
    V(&fill);
  }
  printf("finished \n");
  while(1) ;
}

void Tconsume(void * pc) {
  int i = 0;
  while (i++ < NUM_PARE) {
    P(&fill);
    printf(")%c", *(char*)pc);
    V(&empty);
  }
  printf("finished\n");
  while(1) ;
}

void test_pc_sem(){
  
  kmt->sem_init(&fill, "fill", 0);
  kmt->sem_init(&empty, "empty", 2);
  for(int i = 0; i < NThread; i++){
      kmt->create(pmm->alloc(sizeof(task_t)), "producer", Tproduce, "abcdefgijkl" + i);
      kmt->create(pmm->alloc(sizeof(task_t)), "consumer", Tconsume, "abcdefgh" + i);
  }
}

