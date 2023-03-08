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
  for(; i < 10-1; i++)
  {
    c[i] = fgetc(proc_read_begin);
    if(c[i] == ' ')
      break;
  }
  c[i] = '\0';
  buff->pid = atoi(c);

  /*read the process name*/
  for(i = 0; i < TASK_COMM_LEN-1; i++)
    if((c[i] = fgetc(proc_read_begin)) == ' ')
      break;
  c[i] = '\0';
  strncpy(buff->name, c, TASK_COMM_LEN);

  
  /*read the state*/
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



int main(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]);

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

  for(int i = 0; i < cnt_proc; i++)
  {
    printf("pid: %d, name: %s, ppid: %d\n", processes[i].pid, processes[i].name, processes[i].ppid);

  }
  /*process the info*/
  
  
  return 0;
}
