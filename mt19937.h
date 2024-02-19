#ifndef MT19937_H
#define MT19937_H

#include <stdint.h>

struct MT19937state {
  uint32_t MT[624];
  int index;
};

#define INIT_MT19937state                                                      \
  {                                                                            \
    {0}, (sizeof(((struct MT19937state *)0)->MT) /                             \
          sizeof(*(((struct MT19937state *)0)->MT))) +                         \
             1                                                                 \
  }

void MT19937seed(struct MT19937state *mt, uint32_t seed);
uint32_t MT19937extract(struct MT19937state *mt);

#endif
