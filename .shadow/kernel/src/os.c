#include <common.h>
#include <os.h>
#include <devices.h>

static void print_handlers();

//#define DEBUG_LOCAL0

extern void test_sum();
extern void test_pc_sem();
extern void test_starvation();
extern void test_sched();
extern void test_ctx();
extern void yield_test();
extern void thread_switch_test();

static void test_dev();

static void os_init() {
  print_local("\nthis is cpu[%d]\n", cpu_current());
  pmm->init();
  kmt->init();
  print_handlers();
  print_local("os init finished\n");
  print_local("num cpu: %d\n", cpu_count());

#ifdef LOCAL_DEBUG
  dev->init();
  test_dev();
  //test_pc_sem();
#endif
}

static void os_run() {
  //print_local("os-run executed, hello, n_cpu: %d\n", cpu_count());
  iset(true);
  while (1) {
    assert(ienabled());
    //putch('0');
  }
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
  assert(ienabled() == false);

  enable_last_task(cpu_current());

  save_context(context);

  Context* next_ctx = NULL;
  for(Handler_node* nd = Handlers.next; nd; nd = nd->next){
    if(nd->event == ev.event || nd->event == EVENT_NULL){
      Context* ret_handle = nd->handler(ev, context);
      panic_on(ret_handle && next_ctx, "returning to multiple context!!!\n");
      if(ret_handle) next_ctx = ret_handle;
    }
  }
  panic_report(next_ctx == NULL, "cpu[%d] receives sig: trap ev-no: %d, msg: %s \n", cpu_current(), ev.event, ev.msg);
  assert(ienabled() == false);
  return next_ctx;
}

//push the handler in my handler list
static void os_on_irq(int seq, int event, handler_t handler){
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
  print_local("===handlers: ");
  for(Handler_node* p = Handlers.next; p; p = p->next){
    print_local("[irq: %d, ev: %d] ", p->seq, p->event);
  }
  print_local("===\n");
}

int no_print(const char * fmt, ...){
    return 0;
}

#define STR_BUFFER_SIZE 1024
int vprintf_os(const char *fmt, va_list ap) {
  char formated[STR_BUFFER_SIZE];
  int len = vsnprintf(formated, STR_BUFFER_SIZE, fmt, ap);
  putstr(formated);
  return len;
}
void panic_report(bool cond, const char * fmt, ...){
  if(cond){
    va_list ap;
    va_start(ap, fmt);
    vprintf_os(fmt, ap);
    halt(1);
  }
}

static void tty_reader(void *arg) {
  device_t *tty = dev->lookup(arg);
  char cmd[128], resp[128], ps[16];
  snprintf(ps, 16, "(%s) $ ", arg);
  while (1) {
    tty->ops->write(tty, 0, ps, strlen(ps));
    int nread = tty->ops->read(tty, 0, cmd, sizeof(cmd) - 1);
    cmd[nread] = '\0';
    sprintf(resp, "tty reader task: got %d character(s).\n", strlen(cmd));
    tty->ops->write(tty, 0, resp, strlen(resp));
  }
}

static void test_dev(){
  printf("dev init\n");
  kmt->create(tsk_alloc(), "tty_reader", tty_reader, "tty1");
  kmt->create(tsk_alloc(), "tty_reader", tty_reader, "tty2");
  printf("dev init finished\n");
}

