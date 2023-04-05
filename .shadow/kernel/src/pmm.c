#include <common.h>

struct Heap_node{
  uintptr_t size;
  struct Heap_node* next;
} HEAP_HEAD;
typedef struct Heap_node Heap_node;

void init_heap_node(Heap_node* nd, uintptr_t sz, Heap_node* nxt){
  nd->size = sz;
  nd->next = nxt;
}

#define HEAP_HEAD_SIZE (sizeof(void*) + sizeof(Heap_node*))
#define INIT_HEAP_HEAD(heap_sz) \
HEAP_HEAD.next = heap.start; init_heap_node(HEAP_HEAD.next, heap_sz - HEAP_HEAD_SIZE, NULL)

void* end_of_node(Heap_node* nd){
  return (void*)nd + HEAP_HEAD_SIZE + nd->size;
}

static void *kalloc(size_t size) {
  size_t required_sz = size + HEAP_HEAD_SIZE;
  Heap_node* p, *pre;
  for(pre = &HEAP_HEAD, p = HEAP_HEAD.next; p != NULL; pre = p, p=p->next){
    if(p->size < required_sz){
      continue;
    }
    if(p->size == required_sz){/*just remove the node from list*/
      pre->next = p->next;
      break;
    }
    /*reduce the size of the node*/
    p->size -= required_sz;
    p = end_of_node(p);
  }
  if(p == NULL){
    return NULL;
  }
  return (void*)p + HEAP_HEAD_SIZE;
}

void merge_node(Heap_node* reslt, Heap_node* merged){
  reslt->size += merged->size + HEAP_HEAD_SIZE;
}

static void kfree(void *ptr) {
  /*find the position*/
  uintptr_t freed = (uintptr_t)ptr - HEAP_HEAD_SIZE;

  Heap_node* p, *pre;
  for(pre = &HEAP_HEAD, p = HEAP_HEAD.next; p != NULL; pre = p, p = p->next){
    if(freed < (uintptr_t)p){
      break;
    }
  }
  if(end_of_node((Heap_node*)freed) == p){
    merge_node((Heap_node*)freed, p);
  }
  if((uintptr_t)end_of_node(pre) == freed){
    merge_node(pre, (Heap_node*)freed);
  }
}


static void pmm_init() {
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  assert(pmsize >= HEAP_HEAD_SIZE);
  INIT_HEAP_HEAD(pmsize);
  
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
}

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};
