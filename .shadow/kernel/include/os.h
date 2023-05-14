#ifndef __OS_H__
#define __OS_H__

#include <common.h>

#define CANARY 0xa5a55a5a
#define CANARY_ALIVE(t) (*(uint32_t*)((t)->stack) == CANARY)
#define OS_STACK_SIZE 8192
#define MAX_CPU 16
#define SEM_WAITING_LEN 32

struct spinlock {
  const char * desc;
  int val;
};


typedef enum{ RUNNING, RUNNABLE, INTR } task_stat;
struct task {
  union 
  {
    struct 
    {
      unsigned int  canary1;
      int           id;
      int           cpu;          //if not running, then cpu is -1
      const char*   name;
      task_stat     stat;
      bool          blocked;      
      Context       ctx;
      int           lock;
      unsigned int  canary2;
    };
    uint8_t stack[OS_STACK_SIZE];
  }; 
};
extern task_t * current[MAX_CPU];
extern bool sane_task   (task_t* tsk);

#define curr (current[cpu_current()])

void save_context(Context* ctx);

typedef struct P_task_node {
  task_t* p_task;
  struct P_task_node * next;
} P_task_node;

struct semaphore {
  const char * name;
  int val;
  int lock;
};

task_t* tsk_alloc();

#ifdef LOCAL_DEBUG
    #define print_local printf
#else  
    extern int no_print(const char * fmt, ...);
    #define print_local no_print
#endif

bool cross_check();
void dump_task_info(task_t* tsk);

#endif
