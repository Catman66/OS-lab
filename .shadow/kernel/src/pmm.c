#include <common.h>
#include <mylock.h>

typedef struct Heap_node{
  uintptr_t size;
  struct Heap_node* next;
} Heap_node;

void INIT_NODE(Heap_node* nd, uintptr_t sz, Heap_node* nxt){
  nd->size = sz;
  nd->next = nxt;
}

#define HEAP_HEAD_SIZE        (sizeof(void*) + sizeof(Heap_node*))
#define FREE_SPACE_END(nd)    ((uintptr_t)(nd) + HEAP_HEAD_SIZE + (nd)->size)
#define FREE_SPACE_BEGIN(nd)  ((uintptr_t)(nd) + HEAP_HEAD_SIZE)
#define NODE(ptr)             ((Heap_node*)(ptr))
#define INTP(nd)              ((uintptr_t)(nd))

static void INIT_HEADS();
static void INIT_BOUNDS();

//heap in my test
#ifdef TEST
#define HEAP_SIZE 0x8000000 - 0x300000 
Area heap = {};
#endif

//4 sub-heaps VS concurrency
#define NUM_SUB_HEAP 4
static int cnt = 0;

uintptr_t UPPER_BOUNDS[NUM_SUB_HEAP];
Heap_node HEADS[NUM_SUB_HEAP];

int WHICH_HEAP(void* ptr){
  for(int i = 0; i < NUM_SUB_HEAP; i++){
    if(INTP(ptr) < UPPER_BOUNDS[i]){
      return i;
    }
  }
  printf("which heap error, bounds error or ptr error\n");
  assert(0);
}

//lock 
extern void LOCK(lock_t* lk);
extern void UNLOCK(lock_t* lk);
spinlock_t lk[NUM_SUB_HEAP] = { SPIN_INIT(), SPIN_INIT(), SPIN_INIT(), SPIN_INIT() };
spinlock_t cnt_lk =  SPIN_INIT();

//print --local debug mod
//#define PAINT 1
const char IN_HEAP_TAG  = 0xcc;
const char OUT_HEAP_TAG = 0x0;

void display_space_lst(int hp){
  printf("FREE_LIST: ");
  for(Heap_node* p = HEADS[hp].next; p ; p=p->next){
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
      //print_context(p, p + HEAP_HEAD_SIZE);
      assert(0);
    }
  }
}

void check_free_list(bool after_alloc){
  for(int i = 0; i < NUM_SUB_HEAP; i++){
    for(Heap_node* p = HEADS[i].next; p ; p = p->next){
      if(IN_RANGE((void*)p, heap) && IN_RANGE((void*)(FREE_SPACE_END(p) - 1), heap)){
        continue;
      }
      printf("heap node out of range after %s \n", after_alloc ? "alloc" : "free");
      printf("node at %p:[last = %p, next = %p]", p,  FREE_SPACE_END(p->size), p->next);
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

static void* locked_sub_alloc(int hp, int size){
    assert(hp >= 0 && hp < NUM_SUB_HEAP);
    size_t required_sz = size + HEAP_HEAD_SIZE, round_sz = make_round_sz(size);
    Heap_node* p, * ret_nd;
    uintptr_t ret;

    for(p = HEADS[hp].next; p != NULL; p=p->next){
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
    check_paint(ret_nd, IN_HEAP_TAG);
    paint(ret_nd, OUT_HEAP_TAG);
#endif

    break;
  }
  if(p == NULL){
    return NULL;
  }
  return (void*)ret;
}

static void *kalloc(size_t size) {
  
  int hp;
  LOCK(&cnt_lk);
  hp = cnt++;
  cnt %= NUM_SUB_HEAP;
  UNLOCK(&cnt_lk);

  LOCK(&(lk[hp]));
  void* alloced = locked_sub_alloc(hp, size);
  UNLOCK(&(lk[hp]));

  check_free_list(true);
  return alloced;
}

static void kfree(void *ptr) {
  /*find the position*/
  
  int hp = WHICH_HEAP(ptr);
  
  Heap_node* freed_nd = ptr - HEAP_HEAD_SIZE;
  LOCK(&(lk[hp]));
#ifdef PAINT
  check_paint(freed_nd, OUT_HEAP_TAG);
#endif
  
  Heap_node* p, *pre;
  for(pre = &HEADS[hp], p = HEADS[hp].next; p != NULL; pre = p, p = p->next){
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
#ifdef PAINT
  paint(freed_nd, IN_HEAP_TAG);
#endif
  UNLOCK(&(lk[hp]));
  check_free_list(false);
}

void display_bounds(){
  for(int i = 0; i < NUM_SUB_HEAP; i++){
    printf("%d:[%lx]  ", i, UPPER_BOUNDS[i]);
  }
  printf("\n");
}

static void pmm_init() {
#ifndef TEST   
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  assert(pmsize >= HEAP_HEAD_SIZE);
  
#else
  uintptr_t pmsize = HEAP_SIZE;
  char *ptr  = malloc(HEAP_SIZE);
  heap = (Area){ .start = ptr, .end = ptr + pmsize};

#endif
  INIT_HEADS();
  INIT_BOUNDS();
  printf("Got %ld MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
}

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};

static void INIT_HEADS(){
  uintptr_t sub_hp_sz = sub_hp_sz = (INTP(heap.end) - INTP(heap.start)) / NUM_SUB_HEAP;
  void * nd1 = heap.start;
  for(int i = 0;i < NUM_SUB_HEAP; i++, nd1 += sub_hp_sz){
    HEADS[i].next = nd1;
    INIT_NODE(nd1, sub_hp_sz, NULL);
  }
  //last sub heap 
  Heap_node* lst_hd = HEADS[NUM_SUB_HEAP - 1].next;
  lst_hd->size = INTP(heap.end) - INTP(lst_hd);
}

void INIT_BOUNDS(){
  Heap_node* nd1;
  for(int i = 0; i < NUM_SUB_HEAP; i++){
    nd1 = HEADS[i].next;
    UPPER_BOUNDS[i] = INTP(nd1) + nd1->size; 
  }
}
/*
original version of pmm_init
static void pmm_init() {
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
}
*/