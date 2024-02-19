#include <stdint.h>

#include "mt19937.h"

/* Based on Wikipedia pseudocode
 * https://en.wikipedia.org/wiki/Mersenne_Twister#Pseudocode */

#define w 32
#define n                                                                      \
  (sizeof(((struct MT19937state *)0)->MT) /                                    \
   sizeof(*(((struct MT19937state *)0)->MT)))
#define m 397
#define r 31
#define a 0x9908b0df
#define u 11
#define d 0xffffffff
#define s 7
#define b 0x9d2c5680
#define t 15
#define c 0xefc60000
#define l 18
#define f 1812433253

void MT19937seed(struct MT19937state *mt, uint32_t seed) {
  int i;
  mt->index = n;
  mt->MT[0] = seed;
  for (i = 1; i < n; i++)
    mt->MT[i] = f * (mt->MT[i - 1] ^ (mt->MT[i - 1] >> (w - 2))) + i;
}

static void twist(struct MT19937state *mt) {
  int64_t lowermask = (1L << r) - 1, uppermask = (~lowermask);
  int i;

  for (i = 0; i < n; i++) {
    int64_t x = (mt->MT[i] & uppermask) | (mt->MT[(i + 1) % n] & lowermask),
            xA = x >> 1;
    if (x % 2)
      xA ^= a;
    mt->MT[i] = mt->MT[(i + m) % n] ^ xA;
  }
  mt->index = 0;
}

uint32_t MT19937extract(struct MT19937state *mt) {
  int64_t y;
  if (mt->index >= n) {
    if (mt->index > n)
      MT19937seed(mt, 5489);
    twist(mt);
  }

  y = mt->MT[mt->index++];
  y ^= (y >> u) & d;
  y ^= (y << s) & b;
  y ^= (y << t) & c;
  y ^= y >> l;
  return y;
}
