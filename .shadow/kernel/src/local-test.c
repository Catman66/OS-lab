#include<common.h>
#include<os.h>
#include<devices.h>


extern void test_sum();
extern void test_pc_sem();
extern void test_starvation();
extern void test_sched();
extern void test_ctx();
extern void yield_test();
extern void thread_switch_test();
static void test_dev();

void local_test(test_n t){
    switch (t)
    {
    case T_DEV:
        test_dev();
        break;
    case T_PC:
        test_pc_sem();
        break;
    default:
        panic("error branch in local-test\n");
        break;
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

static void create_dev_task(){
  print_local("dev init\n");
  kmt->create(tsk_alloc(), "tty_reader", tty_reader, "tty1");
  kmt->create(tsk_alloc(), "tty_reader", tty_reader, "tty2");
  print_local("dev init finished\n");
}

static void test_dev(){
    local_test(T_DEV);
    dev->init();
    create_dev_task();
}