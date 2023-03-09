#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <dirent.h>
#include <string.h>
#include <malloc.h>

#ifndef TASK_COMM_LEN
#define TASK_COMM_LEN 16
#endif

#define MAX_NUM_STR_LEN 10
typedef enum
{
  true = 1, false = 0
} bool;

typedef enum
{
  NUM = 1, OTHER = 0
} char_type;

char_type get_char_type(char c)
{
  if(c >= '0' && c <= '9')
    return NUM;
  
  return OTHER;
}

bool is_num(const char* str)
{
  if(*str == '\0')
    return false;

  for(const char* it = str; *it != '\0'; it++)
    if(get_char_type(*it) != NUM)
      return false;
  return true;
}

typedef struct 
{
  char name[TASK_COMM_LEN];
  int pid;
  int ppid;
} Proc_info;

#define DEFAULT_NUM_PROCESS 64
typedef struct
{
  Proc_info* content;
  int size;
  int capacity;
} Process_info_arr;




void init_process_arr(Process_info_arr* p_arr)
{
  p_arr->size = 0;
  p_arr->capacity = DEFAULT_NUM_PROCESS;
  p_arr->content = malloc(sizeof(Proc_info) * p_arr->capacity);
}
void release_space(Process_info_arr* container)
{
  free(container->content);
  container->content = NULL;
  container->capacity = 0;
  container->size = 0;
}

typedef Proc_info item_swap_t;
void swap(item_swap_t* a, item_swap_t* b);
void sort_proc(Process_info_arr* arr)
{
  Proc_info * processes = arr->content;
  int cnt_process = arr->size;

  bool move = false;
  for(int i = 0; i < cnt_process - 1; i++)
  {
    move = false;
    for(int j = 0; j < cnt_process - 1 - i; j++)
      if(processes[j].pid > processes[j+1].pid)
        swap(&processes[j], &processes[j+1]), move = true;
    
    if(move == false)
      break;
  }
}
void add_new_proc(Process_info_arr* curr_processes, Proc_info* info)
{
  if(is_full(curr_processes))
  {
    curr_processes->capacity *= 2;

    Proc_info* new_arr = malloc(sizeof(Proc_info) * curr_processes->capacity);

    for(int i = 0; i < curr_processes->size; i++)
      new_arr[i] = curr_processes->content[i];
    
    free(curr_processes->content);
    curr_processes->content = new_arr;
  }

  curr_processes->content[curr_processes->size++] = *info;
}


int print_width_i(int n)
{
  assert(n >= 0);
  if(n < 10)
    return 1;
  
  if(n < 100)
    return 2;

  if(n < 1000)
    return 3;
  
  if(n < 10000)
    return 4;

  if(n < 100000)
    return 5;
  
  printf("int too large\n");
  assert(0);
}

void read_proc_info(FILE* proc_read_begin, Proc_info* buff)
{
  char c[TASK_COMM_LEN];
  int i = 0;
  for(; i < MAX_NUM_STR_LEN; i++)
  {
    c[i] = fgetc(proc_read_begin);
    if(c[i] == ' ')
      break;
  }
  c[i] = '\0';
  buff->pid = atoi(c);
  
  /*read the process name*/
  // read '('
  assert(fgetc(proc_read_begin) == '(');
  
  char buf_c;
  i = 0;
  while((buf_c = fgetc(proc_read_begin)) != ' ')
  {
    if(i < TASK_COMM_LEN)
      c[i++] = buf_c;
    else
      continue;
  }
  
  c[i-1] = '\0';
  strncpy(buff->name, c, TASK_COMM_LEN);

  
  /*read the 'state' and ' '*/
  fgetc(proc_read_begin);
  fgetc(proc_read_begin);

  /*read the ppid*/
  for(i = 0; i < TASK_COMM_LEN-1; i++)
    if((c[i] = fgetc(proc_read_begin)) == ' ')
      break;
  c[i] = '\0';
  buff->ppid = atoi(c);
}

#define MAX_PATH_LEN 64
#define MAX_PROC_NUM 64

typedef int Item_t;

typedef struct list_node
{
  Item_t val;
  struct list_node* next; 
} list_node;
void insert(list_node* dest, int child);
bool is_empty_list(list_node* head)
{
  return head->next == NULL;
}
void test_list()
{
  list_node* head = malloc(sizeof(list_node));
  for(int i = 0; i < 10; i++)
    insert(head, i);
  
  printf("test list: ");
  for(list_node* p = head->next; p != NULL; p = p->next)
    printf("%d ", p->val);
}

void insert(list_node* dest, int child)
{
  list_node* pre = dest, *p = dest->next;
  
  for(; p != NULL; pre = p, p = p->next)
    ;
  
  pre->next = malloc(sizeof(list_node));
  pre->next->val = child;
  pre->next->next = NULL;
}

void print_version_pstree()
{
  printf("pstree (PSmisc) 23.4\n\
Copyright (C) 1993-2020 Werner Almesberger and Craig Small\n\
\n\
PSmisc comes with ABSOLUTELY NO WARRANTY.\n\
This is free software, and you are welcome to redistribute it under\n\
the terms of the GNU General Public License.\n\
For more information about these matters, see the files named COPYING.\n"
  );
}

bool print_pid_switch = false;
bool sort_switch  = false;
bool show_version_switch = false;

void print_recursively(Proc_info processes[], list_node relation[], int cnt_proc, int curr_i, int size_sp)
{
  //printed this proc
  char printed[TASK_COMM_LEN+MAX_NUM_STR_LEN+2+1];
  strcpy(printed, processes[curr_i].name);
  int len_proc_name = strlen(processes[curr_i].name);
  
  if(print_pid_switch)
  {
    strcat(printed, "(");
    sprintf(&printed[len_proc_name+1],"%d", processes[curr_i].pid);
    strcat(printed, ")");
  }
  printf("%s", printed);
  
  
  //is a leaf proc
  if(is_empty_list(&relation[curr_i]))
    return;
  
  //have at least one child
  printf("-");
  size_sp += (strlen(printed) + 1);
  
  list_node* p = relation[curr_i].next;
  if(p->next != NULL)
    printf("+-");
  else
    printf("--");
  print_recursively(processes, relation, cnt_proc, p->val, size_sp+2);
  printf("\n");
  for(p = p->next; p != NULL; p = p->next)
  {
    for(int i = 0; i < size_sp; i++)
      printf(" ");

    if(p->next == NULL)
      printf("`-");
    else
      printf("|-");
    
    print_recursively(processes, relation, cnt_proc, p->val, size_sp+2);
    
    printf("\n");
  }
  

}

void print_proc_tree(Process_info_arr* processes);
void print_arr(int arr[], int len)
{
  for(int i = 0; i < len; i++)
    printf("%d ", arr[i]);
  printf("\n");
}



void read_proc_files(Process_info_arr* processes)
{
  DIR* proc_dir = opendir("/proc");
  struct dirent* file_in_dir;
  char proc_file_path[MAX_PATH_LEN] = "/proc/";
  Proc_info buff;

  while((file_in_dir = readdir(proc_dir)) != NULL)
  {
    if(is_num(file_in_dir->d_name))
    {
      proc_file_path[6] = '\0';
      strcat(proc_file_path, file_in_dir->d_name);
      strcat(proc_file_path, "/stat");

      /*read the file of path*/
      FILE* p_file = fopen(proc_file_path, "r");
      
      //read the info of a proc
      read_proc_info(p_file, &buff);
      add_new_proc(processes, &buff);
    }
  }
}

void my_pstree()
{
  if(show_version_switch)
  {
    print_version_pstree();
    return;
  }

  Process_info_arr processes;
  init_process_arr(&processes);
  
  read_proc_files(&processes);

  if(sort_switch)
    sort_proc(&processes);
  
  /*process the info*/
  print_proc_tree(&processes);
  release_space(&processes);
}
#include<unistd.h>
#include<getopt.h>

const char * valid_option[] = {"-V", "-p", "-n"};
const struct option valid_long_options[] = 
{
  //name, no-arg:0/arg:1/optional:2
  { "show-pids",     0, NULL, 'p'},
  { "numeric-sort",  0, NULL, 'n'},
  { "version",       0, NULL, 'V'},
  { 0,              0, NULL, '\0'}
};


// struct option //
/*
struct option {
               const char *name;
               int         has_arg;
               int        *flag;
               int         val;
           };

       The meanings of the different fields are:

       name   is the name of the long option.

       has_arg
              is:  no_argument (or 0) if the option does not take an argument; required_argument (or 1) if the option
              requires an argument; or optional_argument (or 2) if the option takes an optional argument.

       flag   specifies how results are returned for a long option.  If flag is NULL, then getopt_long() returns val.
              (For  example,  the  calling program may set val to the equivalent short option character.)  Otherwise,
              getopt_long() returns 0, and flag points to a variable which is set to val if the option is found,  but
              left unchanged if the option is not found.

       val    is the value to return, or to load into the variable pointed to by flag.

*/

int parse_arg(int argc, char* argv[])
{
  char arg_buff;
  while((arg_buff = getopt_long(argc, argv, "Vpn", valid_long_options, NULL)) != -1)
  {
    switch(arg_buff)
    {
      case 'p':
        print_pid_switch = true;
        break;
      case 'V':
        show_version_switch = true;
        break;
      case 'n':
        sort_switch = true;
        break;
      default:
        printf("shouldn't reach here!");
        assert(0);
    }
  }

  return 0;
}

int main(int argc, char *argv[]) {
  int succ = 0;
  succ = parse_arg(argc, argv);
  if(succ != 0)
  {
    printf("error in parsing arguments\n");
    return 0;
  }
  //parse the opts
  my_pstree();

  return 0;
}


void print_proc_tree(Process_info_arr* container)
{
  Proc_info* processes = container->content;
  int cnt_proc = container->size;
  list_node* arr = malloc(sizeof(list_node) * cnt_proc);
  for(int i = 0; i < cnt_proc; i++) arr[i].next = NULL;

  int first_proc = -1;
  for(int i = 0; i < cnt_proc; i++)
  {
    int parent = processes[i].ppid;
    
    int j = 0;
    for(; j < cnt_proc; j++)
      if(processes[j].pid == parent)
      {
        insert(&arr[j], i);
        break;
      }
    if(j == cnt_proc)
      first_proc = i;
  }

  assert(first_proc != -1);

  print_recursively(processes, arr, cnt_proc, first_proc, 0);

}

typedef Proc_info item_swap_t;
void swap(item_swap_t* a, item_swap_t* b)
{
  item_swap_t tmpt = *a;
  *a = *b;
  *b = tmpt;
}

/* used code
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]);
*/