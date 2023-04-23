#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>


#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define STR_BUFFER_SIZE 256
int printf(const char *fmt, ...) {
  va_list para_lst;
  va_start(para_lst, fmt);
  char formated[STR_BUFFER_SIZE];

  size_t len_fmt = strlen(fmt), len_read;
  
  while(len_read = vsnprintf_ret_len(formated, STR_BUFFER_SIZE, fmt, &para_lst) < len_fmt){
    
  }
  
  return 0;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  panic("Not implemented");
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  
}

int vsnprintf_ret_next_idx(char* out, size_t n, const char * fmt, va_list* ap){

}


#endif
