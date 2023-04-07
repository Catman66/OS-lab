#include <common.h>

const char IN_HEAP = 0xcc;
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

#define HEAP_HEAD_SIZE (sizeof(void*) + sizeof(Heap_node*))
#define INIT_HEAP_HEAD(heap_sz) \
HEAP_HEAD.next = heap.start; init_heap_node(HEAP_HEAD.next, heap_sz - HEAP_HEAD_SIZE, NULL)

#define FREE_SPACE_END(nd)    ((uintptr_t)(nd) + HEAP_HEAD_SIZE + (nd)->size)
#define FREE_SPACE_BEGIN(nd)  ((uintptr_t)(nd) + HEAP_HEAD_SIZE)
#define NODE(ptr)             ((Heap_node*)(ptr))
#define INTP(nd)              ((uintptr_t)(nd))

void display_space_lst(){
  for(Heap_node* p = HEAP_HEAD.next; p ; p=p->next){
    printf("[sz: %lx] ", p->size);
  }
  printf("\n");
}

void merge_node(Heap_node* reslt, Heap_node* merged){
  assert(FREE_SPACE_END(reslt) == INTP(merged));

  reslt->size += (merged->size + HEAP_HEAD_SIZE);
  reslt->next = merged->next;
}

void paint(Heap_node* nd, char val){
  memset((void*)FREE_SPACE_BEGIN(nd), val, nd->size);
}

void check_paint(Heap_node* nd, uint8_t val){
  for(char* p = NODE(FREE_SPACE_BEGIN(nd)); p < FREE_SPACE_END(nd); p++){
    if(*p != val){
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
  size_t required_sz = size + HEAP_HEAD_SIZE, round_sz = make_round_sz(size);
  Heap_node* p;
  uintptr_t ret;

  for(p = HEAP_HEAD.next; p != NULL; p=p->next){
    if(required_sz > p->size){
      continue;
    }
    if(ROUNDDOWN(FREE_SPACE_END(p) - size, round_sz) < FREE_SPACE_BEGIN(p) + HEAP_HEAD_SIZE){
      continue; 
    }
    
    ret = ROUNDDOWN(FREE_SPACE_END(p) - size, round_sz);
    Heap_node* ret_nd = (Heap_node*)(ret - HEAP_HEAD_SIZE);
    ret_nd->size = FREE_SPACE_END(p) - ret;
    p->size -= (ret_nd->size + HEAP_HEAD_SIZE);

    check_paint(ret_nd, IN_HEAP);
    paint(ret_nd, OUT_HEAP);
    break;
  }
  if(p == NULL){
    return NULL;
  }
  return (void*)ret;
}


static void kfree(void *ptr) {
  /*find the position*/
  Heap_node* freed_nd = ptr - HEAP_HEAD_SIZE;
  check_paint(freed_nd, OUT_HEAP);
  paint(freed_nd, IN_HEAP);

  Heap_node* p, *pre;
  for(pre = &HEAP_HEAD, p = HEAP_HEAD.next; p != NULL; pre = p, p = p->next){
    if(freed_nd < p){
      break;
    }
  }
  if(FREE_SPACE_END(freed_nd) == INTP(p)){
    merge_node(freed_nd, p);
  }
  else{
    freed_nd->next = p;
  }
  if(FREE_SPACE_END(pre) == INTP(freed_nd)){
    merge_node(pre, freed_nd);
  }
  else{
    pre->next = freed_nd;
  }

  
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
  printf("Got %d MiB heap: [%p, %p)\n", HEAP_SIZE >> 20, heap.start, heap.end);
  printf("display:\n");
  display_space_lst();
  printf("end of display\n");
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