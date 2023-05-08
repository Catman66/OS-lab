#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  const char * p;
  for(p = s; *p ; p++) ;
  return (p - s);
}

char *strcpy(char *dst, const char *src) {
  const char* from = src;
  for(char* to = dst; *from;){
    *(to++) = *(from++);
  }
  return dst;
  
}

char *strncpy(char *dst, const char *src, size_t n) {
  panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
  panic("Not implemented");
}

int strcmp(const char *s1, const char *s2) {
  for(; *s1 || *s2; s1++, s2++){
    if(*s1 < *s2){
      return -1;
    } else if(*s1 > *s2){
      return 1;
    }
  }
  return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  for(int i = 0; i < n && (*(s1+i) || *(s2+i)); i++){
    if(*(s1+i) < *(s2+i)){
      return -1;
    } else if(*(s1+i) > *(s2+i)) {
      return 1;
    }
  }
  return 0;
}

void *memset(void *s, int c, size_t n) {
  char* p = s, * ed = s + n;
  while(p != ed){
    *(p++) = c;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  char * dst = out;
  const char *src = in;
  for(int i = 0; i < n; i++){
    *(dst++) = *(src++);
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  panic("Not implemented");
}

#endif
