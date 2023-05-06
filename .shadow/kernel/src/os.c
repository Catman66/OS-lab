#include <common.h>
#include <os.h>
static void print_handlers();

const char * str = "abcdefg";
void Tprint(void * s){
  while(1){
    printf("%c\n", *(char*)s);
  }
}

void Tprint0(void * a){
  while(1){
    printf("hello from a thread\n");
  }
}

#define DEBUG_LOCAL1

static void os_init() {
  pmm->init();
  kmt->init();
  print_handlers();
  printf("os init finished\n");

#ifdef DEBUG_LOCAL0
  kmt->create(pmm->alloc(sizeof(task_t)), "worker", Tprint0, NULL);
#endif
#ifdef DEBUG_LOCAL1
  for(const char * p = str; *p; p++){
    kmt->create(pmm->alloc(sizeof(task_t)), "printer", Tprint, (void*)p);
  }  
#endif
}

static void os_run() {
  printf("os-run executed, hello\n");
  iset(true);
  while (1) ;
}

//node module
typedef struct Handler_node{
    handler_t handler;
    int seq;
    int event;
    struct Handler_node * next; 
} Handler_node;
static Handler_node* make_new_handler_node(handler_t h, int sq, int ev, Handler_node * nxt){
  Handler_node * made = pmm->alloc(sizeof(Handler_node));
  made->handler = h, made->seq = sq, made->event = ev, made->next = nxt;
  return made;
}
Handler_node Handlers = { .handler = NULL, .seq = 0, .event = 0, .next = NULL }; 

static Context *os_trap(Event ev, Context *context){
  save_context(context);
  Context* next_ctx = NULL;
  for(Handler_node* nd = Handlers.next; nd; nd = nd->next){
    if(nd->event == ev.event || nd->event == EVENT_NULL){
      Context* ret_handle = nd->handler(ev, context);
      panic_on(ret_handle && next_ctx, "returning to multiple context!!!\n");
      if(ret_handle) next_ctx = ret_handle;
    }
  }
  panic_report(next_ctx == NULL, "trap ev-no: %d, msg: %s \n", ev.event, ev.msg);
  //panic_on(!next_ctx, "returning NULL context");
  //panic_on(sane_context(next_ctx), "returning to invalid context");
  return next_ctx;
}

//push the handler in my handler list
static void os_on_irq(int seq, int event, handler_t handler){
  iset(false);      //关中断
  Handler_node* pre, *p;
  for(p = Handlers.next, pre = &Handlers; p; pre = p, p = p->next){
    if(p->seq > seq){
      pre->next = make_new_handler_node(handler, seq, event, p);
      return;
    }
  }
  pre->next = make_new_handler_node(handler, seq, event, NULL);
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
  .trap = os_trap,
  .on_irq = os_on_irq
};

static void print_handlers(){
  printf("===handlers: ");
  for(Handler_node* p = Handlers.next; p; p = p->next){
    printf("[irq: %d, ev: %d] ", p->seq, p->event);
  }
  printf("===\n");
}