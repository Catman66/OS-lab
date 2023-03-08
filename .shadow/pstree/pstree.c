#include <stdio.h>
#include <assert.h>
#include <dirent.h>
#include <string.h>

#ifndef TASK_COMM_LEN
#define TASK_COMM_LEN 16
#endif

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
  char c[10];
  int i = 0;
  for(; i < 10-1; i++)
  {
    c[i] = fgetc(proc_read_begin);
    if(c[i] == ' ')
      break;
  }
  c[i] = '\0';
  printf("%s\n", c);

  int cnt_word = 1;
  while(cnt_word < 3)
  {
    if(fgetc(proc_read_begin) == ' ')
      cnt_word++;
  }
  
  for(i = 0; i < 10-1; i++)
  {
    c[i] = fgetc(proc_read_begin);
    if(c[i] == ' ')
      break;
  }
  c[i] = '\0';
  printf("%s\n", c);

}

#define MAX_PATH_LEN 64
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

  while((file_in_dir = readdir(proc_dir)) != NULL)
  {

    if(is_num(file_in_dir->d_name))
    {
      path[6] = '\0';
      strcat(path, file_in_dir->d_name);
      printf("%s\n", path);
    }
  }

  /*read the info*/
  
  
  return 0;
}
