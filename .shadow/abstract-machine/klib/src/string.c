#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t i = 0;
  for(i = 0; s[i] != '\0'; i++) ;
  return i;
}

char *strcpy(char *dst, const char *src) {
  size_t i = 0;
  while((dst[i] = src[i]) == '\0'){
    i++;
  }
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t i;
  for(i = 0; i < n && src[i] != '\0'; i++){
    dst[i] = src[i];
  }
  for(; i < n; i++){
    dst[i] = '\0';
  }
  return dst;
}

char *strcat(char *dst, const char *src) {
  panic("Not implemented");
}

int strcmp(const char *s1, const char *s2) {
  panic("Not implemented");
}

int strncmp(const char *s1, const char *s2, size_t n) {
  panic("Not implemented");
}

void *memset(void *s, int c, size_t n) {
  char *p = s;
  while(n > 0){
    *(p++) = c;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  size_t i;
  uint8_t* const dst = out;
  const uint8_t* const src = in;
  
  for(i = 0; i < n; i++){
    dst[i] = src[i];
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  panic("Not implemented");
}

#endif