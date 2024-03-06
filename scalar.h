#ifndef SCALAR_H
#define SCALAR_H

typedef float scalar;

#if defined(__ARM_NEON) && !defined(USENEON)
#define USENEON 1
#endif

#if USENEON

#include <arm_neon.h>
typedef float32x4_t scalar4;

#else

typedef struct {
  scalar s[4];
} scalar4;

#endif

scalar4 s4add(scalar4 a, scalar4 b);
scalar4 s4sub(scalar4 a, scalar4 b);
scalar4 s4mul(scalar4 a, scalar4 b);
scalar4 s4div(scalar4 a, scalar4 b);
scalar4 s4load(scalar x);
scalar4 s4loadat(scalar4 a, scalar x, int i);
scalar4 s4abs(scalar4 a);
scalar4 s4sqrt(scalar4 a);
scalar4 s4neg(scalar4 a);
scalar4 s4mulacc(scalar4 a, scalar4 b, scalar4 c);
scalar4 s4mulsub(scalar4 a, scalar4 b, scalar4 c);

scalar s4max(scalar4 a);
scalar s4get(scalar4 a, int i);

#endif
