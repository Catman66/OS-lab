#include <stdio.h>
#include <assert.h>



int main(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]);

  /*read the file of numeric*/
  char *path_to_proc = "/proc/1/stat";
  FILE* proc_reading = fopen(path_to_proc, "r");
  assert(proc_reading != NULL);

  /*read the info*/
  char c[10];
  int i = 0;
  for(; i < 10-1; i++)
  {
    c[i] = fgetc(proc_reading);
    if(c[i] == ' ')
      break;
  }
  c[i] = '\0';
  printf("%s\n", c);

  return 0;
}
