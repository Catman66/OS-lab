#include <common.h>
#include <pthread.h>
// Mutex
typedef pthread_mutex_t mutex_t;
#define MUTEX_INIT() PTHREAD_MUTEX_INITIALIZER
void mutex_lock(mutex_t *lk)   { pthread_mutex_lock(lk); }
void mutex_unlock(mutex_t *lk) { pthread_mutex_unlock(lk); }

mutex_t lk = MUTEX_INIT();

#define PAINT 1
const char IN_HEAP  = 0xcc;
const char OUT_HEAP = 0x0;

struct Heap_node{
  uintptr_t size;
  struct Heap_node* next;
} HEAP_HEAD;
typedef struct Heap_node Heap_node;

void init_heap_node(Heap_node* nd, uintptr_t sz, Heap_node* nxt){
  nd->size = sz;
  nd->next = nxt;
}

#define HEAP_HEAD_SIZE        (sizeof(void*) + sizeof(Heap_node*))
#define FREE_SPACE_END(nd)    ((uintptr_t)(nd) + HEAP_HEAD_SIZE + (nd)->size)
#define FREE_SPACE_BEGIN(nd)  ((uintptr_t)(nd) + HEAP_HEAD_SIZE)
#define NODE(ptr)             ((Heap_node*)(ptr))
#define INTP(nd)              ((uintptr_t)(nd))
#define INIT_HEAP_HEAD(heap_sz) \
HEAP_HEAD.next = heap.start; \
init_heap_node(HEAP_HEAD.next, heap_sz - HEAP_HEAD_SIZE, NULL);\
paint(HEAP_HEAD.next, IN_HEAP)

void display_space_lst(){
  printf("\nFREE_LIST: ");
  for(Heap_node* p = HEAP_HEAD.next; p ; p=p->next){
    printf("[sz: %lx] ", p->size);
  }
  printf("\n");
}

Heap_node* merge_node(Heap_node* reslt, Heap_node* merged){
  assert(FREE_SPACE_END(reslt) == INTP(merged));

  reslt->size += (merged->size + HEAP_HEAD_SIZE);
  reslt->next = merged->next;
  return reslt;
}

void paint(Heap_node* nd, char val){
  memset((void*)FREE_SPACE_BEGIN(nd), val, nd->size);
}

void check_paint(Heap_node* nd, char val){
  for(char* p = (char*)FREE_SPACE_BEGIN(nd); INTP(p) < FREE_SPACE_END(nd); p++){
    if(*p != val){
      printf("===CHECK_PAINT ERROR node: %p,  expected: %x, actual: %x === \n", nd, (uint8_t)val, (uint8_t)*p);
      print_context(p, p + HEAP_HEAD_SIZE);
      assert(0);
    }
  }
}

uintptr_t make_round_sz(size_t sz){
  uintptr_t ret = 1;
  while(ret < sz){
    ret <<= 1;
  }
  return ret;
}

static void *kalloc(size_t size) {
  display_space_lst();
  size_t required_sz = size + HEAP_HEAD_SIZE, round_sz = make_round_sz(size);
  Heap_node* p, * ret_nd;
  uintptr_t ret;
  
  mutex_lock(&lk);
  printf("in\n");
  for(p = HEAP_HEAD.next; p != NULL; p=p->next){
    if(required_sz > p->size){
      continue;
    }
    if(ROUNDDOWN(FREE_SPACE_END(p) - size, round_sz) < FREE_SPACE_BEGIN(p) + HEAP_HEAD_SIZE){
      continue; 
    }
    ret = ROUNDDOWN(FREE_SPACE_END(p) - size, round_sz);
    ret_nd = (Heap_node*)(ret - HEAP_HEAD_SIZE);
    ret_nd->size = FREE_SPACE_END(p) - ret;
    p->size -= (ret_nd->size + HEAP_HEAD_SIZE);

#ifdef PAINT
    check_paint(ret_nd, IN_HEAP);
    paint(ret_nd, OUT_HEAP);
#endif

    break;
  }
  if(p == NULL){
    return NULL;
  }
  printf("out, 0x%lx bytes allocated \n", ret_nd->size);
  display_space_lst();
  mutex_unlock(&lk);
  return (void*)ret;
}


static void kfree(void *ptr) {
  /*find the position*/
  Heap_node* freed_nd = ptr - HEAP_HEAD_SIZE;
  
#ifdef PAINT
  check_paint(freed_nd, OUT_HEAP);
#endif
  mutex_lock(&lk);
  Heap_node* p, *pre;
  for(pre = &HEAP_HEAD, p = HEAP_HEAD.next; p != NULL; pre = p, p = p->next){
    if(freed_nd < p){
      break;
    }
  }
  if(FREE_SPACE_END(freed_nd) == INTP(p)){
    freed_nd = merge_node(freed_nd, p);
  }
  else{
    freed_nd->next = p;
  }
  if(FREE_SPACE_END(pre) == INTP(freed_nd)){
    freed_nd = merge_node(pre, freed_nd);
  }
  else{
    pre->next = freed_nd;
  }
  mutex_unlock(&lk);
#ifdef PAINT
  paint(freed_nd, IN_HEAP);
#endif
}
#ifndef TEST
// 框架代码中的 pmm_init (在 AbstractMachine 中运行)

static void pmm_init() {
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  assert(pmsize >= HEAP_HEAD_SIZE);
  INIT_HEAP_HEAD(pmsize);
  
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
}
#else
// 测试代码的 pmm_init ()
#define HEAP_SIZE 0x8000000 - 0x300000 
Area heap = {};
static void pmm_init() {
  char *ptr  = malloc(HEAP_SIZE);
  heap.start = ptr;
  heap.end   = ptr + HEAP_SIZE;
  INIT_HEAP_HEAD(HEAP_SIZE);
  printf("===Got %d MiB heap: [%p, %p)===\n", HEAP_SIZE >> 20, heap.start, heap.end);
  
}
#endif

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};

/*
original version of pmm_init
static void pmm_init() {
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
}
*/