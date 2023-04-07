#include <kernel.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "thread.h"

#define ROUNDUP(a, sz)      ((((uintptr_t)a) + (sz) - 1) & ~((sz) - 1))
#define ROUNDDOWN(a, sz)    ((((uintptr_t)a)) & ~((sz) - 1))