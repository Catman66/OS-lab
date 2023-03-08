#include <stdio.h>
#include <assert.h>
#include <dirent.h>

#ifndef TASK_COMM_LEN
#define TASK_COMM_LEN 16
#endif

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

int main(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]);

  /*read the file of numeric*/
  DIR* proc_dir = opendir("/proc");
  struct dirent* file_in_dir;
  while((file_in_dir = readdir(proc_dir)) != NULL)
  {
    if(is_numeric_str(file_in_dir->d_name))
    {
      char path[] = "/proc";
      strcat(path, file_in_dir->d_name);
      printf("%s\n", path);
    }
  }

  /*read the info*/
  
  
  return 0;
}
