#include <kernel.h>
#ifndef TEST
#include <klib.h>
#else
#include <stdlib.h>
#endif

int main() {
  ioe_init();
  cte_init(os->trap);
  os->init();
  mpe_init(os->run);
  return 1;
}

