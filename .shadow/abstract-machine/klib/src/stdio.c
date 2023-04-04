#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>


#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define ADD_HEX_PREFIX(num_buffer) *((num_buffer)++) = '0', *((num_buffer)++) = 'x'
#define ADD_SUFFIX_NULL(num_buffer) (*((num_buffer)++) = '\0')


#define PRINTF_BUFFER_SIZE 256
int printf(const char *fmt, ...) {
  va_list p_lst;
  va_start(p_lst, fmt);
  
  char buff[PRINTF_BUFFER_SIZE];
  int ret = vsnprintf(buff, PRINTF_BUFFER_SIZE, fmt, p_lst);
  
  va_end(p_lst);
  putstr(buff);
  return ret;
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

void put_err(){
  putch('\n'), putch('E'), putch('\n');
}

Specification_t parse_type_renew_i(const char* fmt, size_t* _pos){
  if(fmt[(*_pos)++] != '%'){
    put_err();
    return ERROT_s;
  }
  switch(fmt[*_pos]){
    case 'i':
    case 'd':
      return INT_s;
    case 'c':
      return CHAR_s;
    case 's':
      return STR_s;
    case 'x':
      return OCT_INT_s;
    case 'p':
      return PTR_s;
    default:
      return ERROT_s;
  }
}
int converse_dump(char* out, va_list ap, Specification_t tp){
  int num;
  switch (tp)
  {
  case INT_s:
    num = va_arg(ap, int);
    int len_num = utoa(num, out, BASE_DEC);
    return len_num;
  case CHAR_s:
    char c = va_arg(ap, int);
    *out = c;
    return 1;
  case STR_s:
    strcpy(out, va_arg(ap, char* ));
    return strlen(out);
  case OCT_INT_s:
    num = va_arg(ap, uint32_t);
    return utoa(num, out, BASE_HEX);
  case PTR_s:
    uintptr_t ptr_val = (unsigned long)va_arg(ap, uintptr_t);
    return utoa(ptr_val, out, BASE_HEX);
  default:
    return 0x7fffffff;
  }
}


int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  //details here 
  size_t i_fmt, i_out;
  Specification_t tp;
  for(i_fmt = 0, i_out = 0; i_fmt < n && fmt[i_fmt]; i_fmt++){
    switch(fmt[i_fmt]){
      case '%':
        tp = parse_type_renew_i(fmt, &i_fmt);
        if(tp == ERROT_s){
          put_err();
          return -1;
        }
        i_out += converse_dump(out + i_out, ap, tp);
        break;
      default :
        out[i_out++] = fmt[i_fmt];
        break;
    }
  }
  if(i_fmt < n){
    out[i_out++] = '\0';
  }
  return i_out;
}

void puts(const char* msg){
  for(const char* p = msg; *p; p++){
    putch(*p);
  }
}

char itoc(int n){
  if(n >= 10){
    return 'a' + (n-10);
  }
  return n + '0';
}

//stack operations
#define PUSH(st, tp, v) (st)[++(tp)] = (v);
#define POP(st, tp) ((st)[(tp)--])
#define STK_EMPTY(tp) ((tp) == -1)
#define STK_SZ(tp) ((tp)+1)
#define ST_OVERFLOW(tp) (STK_SZ((tp)) > CONVERSION_BUFFER_SIZE)

int utoa(uint64_t num, char* out, itoa_BASE b){
  char* pos_before = out;
  if(b == BASE_HEX){
    ADD_HEX_PREFIX(out);
  }
  if(num == 0){
    *(out++) = '0';
    ADD_SUFFIX_NULL(out);
    return out - pos_before - 1;
  }
  char stk[CONVERSION_BUFFER_SIZE] = {'\0'};
  int top = -1;

  while(num > 0){
    PUSH(stk, top, itoc(num % b));
    num /= b;
  }
   
  while(!STK_EMPTY(top)){
    *(out++) = POP(stk, top);
  }
  ADD_SUFFIX_NULL(out);
  return out - pos_before - 1;
}

int itoa(int64_t num, char* out, itoa_BASE bs){
  if(num > 0){
    return utoa(num, out, bs);
  }
  *(out++) = '-';
  return utoa(abs64(num), out, bs) + 1;
}

#endif
