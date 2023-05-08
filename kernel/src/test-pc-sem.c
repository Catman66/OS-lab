#include <common.h>
#include <os.h>

sem_t fill, empty;
#define NThread 4
#define P kmt->sem_wait
#define V kmt->sem_signal

void Tproduce() {
  while (1) {
    P(&empty);
    printf("(");
    V(&fill);
  }
}

void Tconsume() {
  while (1) {
    P(&fill);
    printf(")");
    V(&empty);
  }
}

void test_pc_sem(){
    kmt->sem_init(&fill, "fill", 0);
    kmt->sem_init(&empty, "empty", 2);
    for(int i = 0; i < NThread; i++){
        kmt->create(pmm->alloc(sizeof(task_t)), "producer", Tproduce, NULL);
        kmt->create(pmm->alloc(sizeof(task_t)), "consumer", Tconsume, NULL);
    }
}

