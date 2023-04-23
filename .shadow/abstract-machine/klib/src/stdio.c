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
  int len = vsnprintf(formated, STR_BUFFER_SIZE, fmt, para_lst);
  va_end(para_lst);
  putstr(formated);
  return len;
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

void reverse(char* buf, int len){
  for(int i = 0; i < len / 2; i++){
    buf[i] = buf[len - 1 - i];
  }
}
char ntoc(int n){
  if(n < 10){
    return '0' + n;
  }
  return n + 'a';
}
int ntoa(char* buf, int n, int base){
  if(n == 0){
    *buf = '0';
    return 1;
  }
  char * buf_base = buf;
  while(n > 0){
    *(buf++) = ntoc(n % base);
    n /= base;
  }
  reverse(buf_base, buf - buf_base);
  *buf = '\0';
  return buf - buf_base;
}
int itoa(char * buf, int n){
  return ntoa(buf, n, 10);
}
int htoa(char * buf, int n){
  *(buf++) = '0';
  *(buf++) = 'x';
  return 2 + ntoa(buf, n, 16);
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  char buffer[CONVERSION_BUFFER_SIZE];
  char * dst = out;
  
  while(out - dst < n && *fmt){
    if(*fmt != '%'){
      *(out++) = *(fmt++);
      continue;
    }
    fmt++;
    char * mov_from = NULL;
    int len = 0;
    switch(*(fmt++)){
      case 'd':
        len = itoa(buffer, va_arg(ap, int));
        mov_from = buffer;
        break;
      case 'p':
        len = htoa(buffer, va_arg(ap, intptr_t));
        mov_from = buffer;
      case 's':
        mov_from = va_arg(ap, char*);
        len = strlen(mov_from);
      default:
        putch(*fmt);
        panic_on(0, " type not implemented\n");
    }
    panic_on(out - dst + len > n, "too long string in my vsnprintf\n");
    out += len;
    strcpy(out, mov_from);
  }
  return out - dst;
}


#endif
