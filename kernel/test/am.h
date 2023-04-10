#ifndef AM_H__
#define AM_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>



typedef struct {
  void *start, *end;
} Area;
extern Area heap;

#define IN_RANGE(ptr, area) ((area).start <= (ptr) && (ptr) < (area).end)

#endif