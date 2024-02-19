#include "random.h"
#include "mt19937.h"
#include "util.h"

#include <pthread.h>
#include <stdlib.h>

pthread_key_t randomkey;

void randominit(void) { assert(!pthread_key_create(&randomkey, 0)); }

float randomfloat(void) {
  struct MT19937state *mt = pthread_getspecific(randomkey);

  if (!mt) {
    assert(mt = malloc(sizeof(*mt)));
    mt->index = nelem(mt->MT) + 1;
    MT19937seed(mt, random());
    assert(!pthread_setspecific(randomkey, mt));
  }

  return (float)MT19937extract(mt) / (float)(1L << 32);
}
