#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>


#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define STR_BUFFER_SIZE 1024
#define CONVERSION_BUFFER_SIZE 64

int printf(const char *fmt, ...) {
  va_list para_lst;
  va_start(para_lst, fmt);
  char formated[STR_BUFFER_SIZE];

  size_t len_fmt = strlen(fmt), len_read;
  
  while(len_read = vsnprintf_ret_len(formated, STR_BUFFER_SIZE, fmt, &para_lst) < len_fmt){
    fmt += len_read, len_fmt -= len_read;
  }
  va_end(para_lst);
  return len_fmt;
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
  char buffer[CONVERSION_BUFFER_SIZE];
  char * dst = out;
  
  int len;
  while(out - dst < n && *fmt){
    if(*fmt != '%'){
      *(out++) = *(fmt++);
      continue;
    }
    fmt++;
    char * mov_from = NULL;
    switch(*(fmt++)){
      case 'd':
        len = itoa(buffer, va_arg(*ap, int));
        mov_from = buffer;
        break;
      case 'p':
        len = htoa(buffer, va_arg(*ap, intptr_t));
        mov_from = buffer;
      case 's':
        mov_from = va_arg(*ap, char*);
        len = strlen(mov_from);
      default:
        putch(*fmt);
        panic_on(0, " type not implemented\n");
    }
    panic_on(out - dst + len > n, "too long string in my vsnprintf\n");
    out += len;
    strcpy(out, mov_from);
  }

  
}


#endif
