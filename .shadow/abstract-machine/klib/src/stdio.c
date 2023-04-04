#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>


#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define PRINTF_BUFFER_SIZE 256
int printf(const char *fmt, ...) {
  va_list p_lst;
  va_start(p_lst, fmt);
  
  char buff[PRINTF_BUFFER_SIZE];
  int ret = vsnprintf(buff, PRINTF_BUFFER_SIZE, fmt, p_lst);

  va_end(p_lst);
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

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  //details here 
  size_t i_fmt, i_out;
  for(i_fmt = 0, i_out = 0; i_fmt < n && fmt[i_fmt]; i_fmt++){
    switch(fmt[i_fmt]){
      case '%':
        //read the length, var type
        //conversion the value, to string
        //append, update i_fmt & i_out
        
      default :
        out[i_out++] = fmt[i_fmt];
    }
  }

  return i_out;
}

#endif
