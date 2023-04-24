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

static void reverse(char* buf, int len){
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
int ntoa(char * buf, long long n, int base);
int itoa(char * buf, long long n);
int htoa(char * buf, long long n);

typedef enum {
  INT_ct, PTR_ct, UINT_ct, STR_ct, LI_ct, LU_ct, LLI_ct, LLU_ct
} CONVERSION_TYPE;
int parse_type(const char * fmt, CONVERSION_TYPE* tp);
int parse_prefix_long(const char * fmt, CONVERSION_TYPE* tp);
int parse_prefix_ll(const char * fmt, CONVERSION_TYPE * tp);

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  char buffer[CONVERSION_BUFFER_SIZE];
  char * dst = out;
  
  while(out - dst < n && *fmt){
    if(*fmt != '%'){
      *(out++) = *(fmt++);
      continue;
    }
    fmt++;  //passs the '%'
    char * mov_from = buffer;
    int out_len = 0, parse_off = 0;
    CONVERSION_TYPE tp;
    parse_off = parse_type(fmt, &tp);
    fmt += parse_off;
    switch(tp){
      case INT_ct:
        out_len = itoa(buffer, va_arg(ap, int));
        break;
      case PTR_ct:
        out_len = htoa(buffer, va_arg(ap, intptr_t));
        break;
      case STR_ct:
        mov_from = va_arg(ap, char*);
        out_len = strlen(mov_from);
        break;
      case LI_ct:
        out_len = itoa(buffer, va_arg(ap, long int));
        break;
      case LLI_ct:
        out_len = itoa(buffer, va_arg(ap, long long int));
        break;
      case LU_ct:
        out_len = itoa(buffer, va_arg(ap, unsigned long));
        break;
      case LLU_ct:
        out_len = itoa(buffer, va_arg(ap, unsigned long long));
        break;
      default:
        putch(*fmt);
        panic_on(0, " type not implemented\n");
    }
    panic_on(out - dst + out_len > n, "too long string in my vsnprintf\n");
    out += out_len;
    strcpy(out, mov_from);
  }
  return out - dst;
}

int ntoa(char* buf, long long n, int base){
  panic_on(n < 0, "htoa apply to a negative number\n");
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
int itoa(char * buf, long long n){
  return ntoa(buf, n, 10);
}
int htoa(char * buf, long long n){
  *(buf++) = '0';
  *(buf++) = 'x';
  return 2 + ntoa(buf, n, 16);
}

int parse_type(const char * fmt, CONVERSION_TYPE* tp){
  int len = 0;
  switch(*(fmt++)){
    case 'd':
    case 'i':
      *tp = INT_ct;
      return 1;
    case 's':
      *tp = STR_ct;
      return 1;
    case 'p':
      *tp = PTR_ct;
      return 1;
    
    case 'l':
      if(*fmt != 'l'){
        len += (1 + parse_prefix_long(fmt, tp));
      } else {  //*fmt == l
        len += (2 + parse_prefix_ll(fmt + 1, tp));
      }
      return len;
    default:
      putch(*(fmt - 1));
      panic("not implemented type\n");
      return -1;
  }
}
int parse_prefix_long(const char * fmt, CONVERSION_TYPE* tp){
  switch(*fmt){
    case 'd':
    case 'i':
      *tp = LI_ct;
      return 1;
    case 'u':
      *tp = LU_ct;
      return 1;
    default:
      panic("parse with prefix long error type");
      return -1;
  }
}
int parse_prefix_ll(const char * fmt, CONVERSION_TYPE * tp){
  switch(*fmt){
    case 'd':
    case 'i':
      *tp = LLI_ct;
      return 1;
    case 'u':
      *tp = LLU_ct;
      return 1;
    default:
      panic("parse with prefix long error type");
      return -1;
  }
}

#endif
