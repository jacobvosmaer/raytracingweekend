#include "scalar.h"

#include <math.h>

#if USENEON

scalar4 s4add(scalar4 a, scalar4 b) { return vaddq_f32(a, b); }
scalar4 s4sub(scalar4 a, scalar4 b) { return vsubq_f32(a, b); }
scalar4 s4mul(scalar4 a, scalar4 b) { return vmulq_f32(a, b); }
scalar4 s4div(scalar4 a, scalar4 b) { return vdivq_f32(a, b); }
scalar4 s4load(scalar x) {
  scalar4 a;
  a[0] = x;
  a[1] = x;
  a[2] = x;
  a[3] = x;
  return a;
}

scalar4 s4loadat(scalar4 a, scalar x, int i) {
  a[i] = x;
  return a;
}

scalar4 s4abs(scalar4 a) { return vabsq_f32(a); }
scalar4 s4sqrt(scalar4 a) { return vsqrtq_f32(a); }
scalar4 s4mulacc(scalar4 a, scalar4 b, scalar4 c) { return vmlaq_f32(a, b, c); }
scalar s4get(scalar4 a, int i) { return a[i]; }
scalar s4max(scalar4 a) { return vmaxvq_f32(a); }

#else

scalar4 s4add(scalar4 a, scalar4 b) {
  int i;
  for (i = 0; i < 4; i++)
    a.s[i] += b.s[i];
  return a;
}

scalar4 s4sub(scalar4 a, scalar4 b) {
  int i;
  for (i = 0; i < 4; i++)
    a.s[i] -= b.s[i];
  return a;
}

scalar4 s4mul(scalar4 a, scalar4 b) {
  int i;
  for (i = 0; i < 4; i++)
    a.s[i] *= b.s[i];
  return a;
}

scalar4 s4div(scalar4 a, scalar4 b) {
  int i;
  for (i = 0; i < 4; i++)
    a.s[i] /= b.s[i];
  return a;
}

scalar4 s4load(scalar x) {
  scalar4 a;
  int i;
  for (i = 0; i < 4; i++)
    a.s[i] = x;
  return a;
}

scalar4 s4loadat(scalar4 a, scalar x, int i) {
  a.s[i] = x;
  return a;
}

scalar4 s4abs(scalar4 a) {
  int i;
  for (i = 0; i < 4; i++)
    a.s[i] = fabsf(a.s[i]);
  return a;
}

scalar4 s4sqrt(scalar4 a) {
  int i;
  for (i = 0; i < 4; i++)
    a.s[i] = sqrtf(a.s[i]);
  return a;
}

scalar4 s4mulacc(scalar4 a, scalar4 b, scalar4 c) {
  return s4add(a, s4mul(b, c));
}
scalar s4get(scalar4 a, int i) { return a.s[i]; }

scalar s4max(scalar4 a) {
  int i;
  for (i = 1; i < 4; i++)
    if (a.s[i] > a.s[0])
      a.s[0] = a.s[i];
  return a.s[0];
}

#endif
