#include <common.h>
#include <mylock.h>

typedef struct Heap_node{
  uintptr_t size;
  struct Heap_node* next;
} Heap_node;

#define HEAP_HEAD_SIZE        (sizeof(void*) + sizeof(Heap_node*))
#define FREE_SPACE_END(nd)    ((uintptr_t)(nd) + HEAP_HEAD_SIZE + (nd)->size)
#define FREE_SPACE_BEGIN(nd)  ((uintptr_t)(nd) + HEAP_HEAD_SIZE)
#define NODE(ptr)             ((Heap_node*)(ptr))
#define INTP(nd)              ((uintptr_t)(nd))

static void INIT_SIMPLE_HPS();
static void INIT_BOUNDS();

void INIT_NODE(void* nd, uintptr_t sz, Heap_node* nxt){
  NODE(nd)->size = sz - HEAP_HEAD_SIZE;
  NODE(nd)->next = nxt;
}
Heap_node* merge_node(Heap_node* reslt, Heap_node* merged){
  assert(FREE_SPACE_END(reslt) == INTP(merged));
  reslt->size += (merged->size + HEAP_HEAD_SIZE);
  reslt->next = merged->next;
  return reslt;
}
//heap in my test
#ifdef TEST
#define HEAP_SIZE (1 << 27)
Area heap = {};
#endif

//4 sub-heaps VS concurrency
#define NUM_SIMPLE_SUB_HP 4
#define NUM_SUB_HEAP (NUM_SIMPLE_SUB_HP + 1)
static int cnt = 0;

uintptr_t UPPER_BOUNDS[NUM_SIMPLE_SUB_HP];
Heap_node HEADS[NUM_SIMPLE_SUB_HP];

int which_simple_heap(void* ptr){
  for(int i = 0; i < NUM_SIMPLE_SUB_HP; i++){
    if(INTP(ptr) < UPPER_BOUNDS[i]){
      return i;
    }
  }
  printf("which heap error, ptr: %p\n", ptr);
  assert(0);
}

//lock 
extern void LOCK(lock_t* lk);
extern void UNLOCK(lock_t* lk);
spinlock_t lk[NUM_SIMPLE_SUB_HP];
spinlock_t cnt_lk = SPIN_INIT();

//print --local debug mod
//#define PAINT 1
const char IN_HEAP_TAG  = 0xcc;
const char OUT_HEAP_TAG = 0x0;

void display_space_lst(int hp);
void display_bounds();
void check_free_list(bool after_alloc);

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

uintptr_t make_round_sz(size_t sz){
  uintptr_t ret = 1;
  while(ret < sz){
    ret <<= 1;
  }
  return ret;
}

//page route
#define PAGE_SIZE           4096
#define NUM_PG_HP           4
#define MAX_POSSIBLE_PRE_PG (1 << 15) 
#define PAGE_HEAP_START     (UPPER_BOUNDS[NUM_SIMPLE_SUB_HP - 1])
int     PG_HP_SIZE;

lock_t pgcnt_lk = SPIN_INIT();
lock_t pg_lks[NUM_PG_HP];
int   pgcnt = 0;
int   last_pg[NUM_PG_HP];
int   n_pg_left[NUM_PG_HP];
bool  pg_tags[MAX_POSSIBLE_PRE_PG] = { 0 };
bool* tag_hp_pg[NUM_PG_HP];

void INIT_PG_HEAPS(uintptr_t st, uintptr_t ed){
  PG_HP_SIZE = (ed - st) / PAGE_SIZE / NUM_PG_HP;
  for(int i = 0; i < NUM_PG_HP; i++){
    last_pg[i]  = 0;
    n_pg_left[i] = PG_HP_SIZE;
    tag_hp_pg[i] = &pg_tags[i * PG_HP_SIZE];
  }
}
void DIVIDE_INIT(){
  uintptr_t sz_all_pghp = ROUNDDOWN((INTP(heap.end) - INTP(heap.start)) / 4, PAGE_SIZE * NUM_PG_HP);
  uintptr_t pghp_start = ROUNDDOWN(INTP(heap.end) - sz_all_pghp, PAGE_SIZE);
  
  INIT_PG_HEAPS(pghp_start, INTP(heap.end));
  INIT_SIMPLE_HPS(INTP(heap.start), pghp_start);
}
#define ALLOCATED 1
#define FREE      0
static int locked_pg_alloc_in(int hp){
  bool* tag_of = tag_hp_pg[hp];
  int*  pg = &last_pg[hp];
  while(tag_of[*pg] == ALLOCATED){
    (*pg)++;
    (*pg) %= PG_HP_SIZE;
  }
  tag_of[*pg] = ALLOCATED;
  n_pg_left[hp]--;
  return *pg;
}
int idx(int hp, int i){
  return hp * PG_HP_SIZE + i;
}
void* to_pg(int hp, int i){
  return (void*)(PAGE_HEAP_START + idx(hp, i) * PAGE_SIZE);
}

static void* pg_alloc(){
  int hp;
  LOCK(&pgcnt_lk);
  hp = pgcnt;
  pgcnt++, pgcnt %= NUM_PG_HP;
  UNLOCK(&pgcnt_lk);

  LOCK(&pg_lks[hp]);
  if(n_pg_left[hp] == 0){
    UNLOCK(&pg_lks[hp]);
    return NULL;
  }
  int idx_pg = locked_pg_alloc_in(hp);
  UNLOCK(&pg_lks[hp]);
  void * pg_allocated = to_pg(hp, idx_pg);
  return pg_allocated;
}

static void* locked_simple_alloc(int hp, int size){
  assert(hp >= 0 && hp < NUM_SIMPLE_SUB_HP);
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
  paint(ret_nd, IN_HEAP_TAG);
  paint(ret_nd, OUT_HEAP_TAG);
#endif

  break;
  }
  if(p == NULL){
    return NULL;
  }
  return (void*)ret;
}

void* simple_alloc(size_t size){
  int hp;
  LOCK(&cnt_lk);
  hp = cnt++;
  cnt %= NUM_SIMPLE_SUB_HP;
  UNLOCK(&cnt_lk);

  LOCK(&(lk[hp]));
  void * alloced = locked_simple_alloc(hp, size);
  UNLOCK(&(lk[hp]));

  return alloced;
}

static void *kalloc(size_t size) {
  if(size == PAGE_SIZE){
    return pg_alloc();
  }
  return simple_alloc(size);
}
int pg_to_idx(void *ptr){
  return (INTP(ptr) - PAGE_HEAP_START) >> 12;
}

void pg_free(void *ptr){
  int idx = pg_to_idx(ptr), hp, pg;
  hp = idx / PG_HP_SIZE, pg = idx % PG_HP_SIZE;
  //printf("free pg idx : %d\n", idx);
  assert(tag_hp_pg[hp][pg] == ALLOCATED);
  LOCK(&pg_lks[hp]);
  tag_hp_pg[hp][pg] = FREE;
  n_pg_left[hp]++;
  last_pg[hp] = pg;
  UNLOCK(&pg_lks[hp]);
}

static void kfree(void *ptr) {
  if(INTP(ptr) >=  PAGE_HEAP_START){
    pg_free(ptr);
    return;
  }
  int hp = which_simple_heap(ptr);
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
}

static void pmm_init() {
#ifndef TEST   
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  assert(pmsize >= HEAP_HEAD_SIZE);
  
#else
  uintptr_t pmsize = HEAP_SIZE;
  char *ptr  = malloc(HEAP_SIZE);
  assert(ptr != NULL);
  heap = (Area){ .start = ptr, .end = ptr + pmsize};
  
#endif
  DIVIDE_INIT();
  INIT_BOUNDS();
  printf("Got %ld MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
  //display_bounds();
}

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};

static void INIT_SIMPLE_HPS(uintptr_t st, uintptr_t ed){
  uintptr_t sub_hp_sz = (INTP(ed) - INTP(st)) / NUM_SIMPLE_SUB_HP;
  void * nd1 = heap.start;
  for(int i = 0;i < NUM_SIMPLE_SUB_HP; i++, nd1 += sub_hp_sz){
    HEADS[i].next = nd1;
    INIT_NODE(nd1, sub_hp_sz, NULL);
  }
  //last sub heap 
  Heap_node* last_nd1 = HEADS[NUM_SIMPLE_SUB_HP - 1].next;
  last_nd1->size = ed - FREE_SPACE_BEGIN(last_nd1);
}

void INIT_BOUNDS(){
  Heap_node* nd1;
  for(int i = 0; i < NUM_SIMPLE_SUB_HP; i++){
    nd1 = HEADS[i].next;
    UPPER_BOUNDS[i] = FREE_SPACE_END(nd1);
  }
}
/*
original version of pmm_init
static void pmm_init() {
  uintptr_t pmsize = ((uintptr_t)heap.end - (uintptr_t)heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, heap.start, heap.end);
}
*/
//printer
void display_space_lst(int hp){
  printf("FREE_LIST: ");
  for(Heap_node* p = HEADS[hp].next; p ; p=p->next){
    printf("[sz: %lx] ", p->size);
  }
  printf("\n");
}

void display_bounds(){
  for(int i = 0; i < NUM_SIMPLE_SUB_HP; i++){
    printf("%d:[%lx]  ", i, UPPER_BOUNDS[i]);
  }
  printf("\n");
}


void check_free_list(bool after_alloc){
  for(int i = 0; i < NUM_SIMPLE_SUB_HP; i++){
    for(Heap_node* p = HEADS[i].next; p ; p = p->next){
      if(IN_RANGE((void*)p, heap) && IN_RANGE((void*)(FREE_SPACE_END(p) - 1), heap)){
        continue;
      }
      printf("heap node out of range after %s \n", after_alloc ? "alloc" : "free");
      printf("node at %p:[last = %p, next = %p]", p, (void*)FREE_SPACE_END(p), p->next);
      assert(0);
    }
  }
}