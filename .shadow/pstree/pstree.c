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
bool is_empty(list_node* head)
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

void print_recursively(Proc_info Proc_info[], list_node relation[], int cnt_proc, int curr_i, int size_sp)
{
  //is a leaf proc
  if(is_empty(&relation[curr_i]))
  {
    printf("%s", Proc_info[curr_i].name);
    return;
  }
  
  printf("%s", Proc_info[curr_i].name);
  printf("-");

  size_sp += (strlen(Proc_info[curr_i].name) + 1);
  
  list_node* p = relation[curr_i].next;
  if(p->next != NULL)
    printf("+-");
  else
    printf("--");
  print_recursively(Proc_info, relation, cnt_proc, p->val, size_sp+2);
  printf("\n");
  for(p = p->next; p != NULL; p = p->next)
  {
    for(int i = 0; i < size_sp; i++)
      printf(" ");

    if(p->next == NULL)
      printf("+-");
    else
      printf("`-");
    
    print_recursively(Proc_info, relation, cnt_proc, p->val, size_sp+2);
    
    printf("\n");
  }
  

}

void print_proc_tree(Proc_info process[], int cnt_proc)
{
  list_node* arr = malloc(sizeof(list_node) * cnt_proc);
  for(int i = 0; i < cnt_proc; i++) arr[i].next = NULL;

  int first_proc = -1;
  for(int i = 0; i < cnt_proc; i++)
  {
    int parent = process[i].ppid;
    
    int j = 0;
    for(; j < cnt_proc; j++)
      if(process[j].pid == parent)
      {
        insert(&arr[j], i);
        break;
      }
    if(j == cnt_proc)
      first_proc = i;
  }
  assert(first_proc != -1);

  print_recursively(process, arr, cnt_proc, first_proc, 0);

}

void my_pstree()
{

  /*read the file of numeric*/
  DIR* proc_dir = opendir("/proc");
  struct dirent* file_in_dir;

  char path[MAX_PATH_LEN] = "/proc/";
  Proc_info *processes = malloc(sizeof(Proc_info)*MAX_PROC_NUM);
  int cnt_proc = 0, capacity = MAX_PATH_LEN;


  while((file_in_dir = readdir(proc_dir)) != NULL)
  {
    if(is_num(file_in_dir->d_name))
    {
      path[6] = '\0';
      strcat(path, file_in_dir->d_name);
      strcat(path, "/stat");

      /*read the file of path*/
      FILE* p_file = fopen(path, "r");
      
      if(cnt_proc == capacity)
      {
        capacity *= 2;

        Proc_info* new_arr = malloc(sizeof(Proc_info) * capacity);

        for(int i = 0; i < cnt_proc; i++)
          new_arr[i] = processes[i];
        
        free(processes);
        processes = new_arr;

      }
      read_proc_info(p_file, &processes[cnt_proc++]);

      
    }
  }

  /*process the info*/
  print_proc_tree(processes, cnt_proc);
  free(processes);
}

int main(int argc, char *argv[]) {


  my_pstree();
  
  return 0;
}


/* used code
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]);
*/