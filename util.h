#ifndef UTIL_H
#define UTIL_H

#define nelem(x) (sizeof(x) / sizeof(*(x)))
#define endof(x) ((x) + nelem(x))
#define assert(x)                                                              \
  if (!(x))                                                                    \
  __builtin_trap()

#endif
