#ifndef VEC3_H
#define VEC3_H

#if defined(__x86_64__) && !defined(USESSE)
#define USESSE 0
#endif

#if defined(__ARM_NEON) && !defined(USENEON)
#define USENEON 1
#endif

#if USENEON

#include <arm_neon.h>
typedef float32x4_t vec3;
typedef float scalar;
typedef float32x4_t scalar4;

#elif USESSE

#include <immintrin.h>
typedef __m128 vec3;
typedef float scalar;

#else

typedef float scalar;
typedef struct {
  scalar x, y, z;
} vec3;

typedef struct {
  scalar s[4];
} scalar4;

#endif

typedef struct {
  scalar4 x, y, z;
} vec3x4;

scalar v3x(vec3 v);
scalar v3y(vec3 v);
scalar v3z(vec3 v);

vec3 v3(scalar x, scalar y, scalar z);
vec3 v3add(vec3 v, vec3 w);
vec3 v3sub(vec3 v, vec3 w);
vec3 v3neg(vec3 v);
vec3 v3mul(vec3 v, vec3 w);
vec3 v3scale(vec3 v, scalar c);
scalar v3dot(vec3 v, vec3 w);
scalar v3length(vec3 v);
vec3 v3unit(vec3 v);
vec3 v3cross(vec3 v, vec3 w);
vec3 v3random(void);
vec3 v3randominterval(scalar min, scalar max);
vec3 v3randomunit(void);
vec3 v3randominunitdisk(void);
int v3nearzero(vec3 v);

scalar4 s4add(scalar4 a, scalar4 b);
scalar4 s4sub(scalar4 a, scalar4 b);
scalar4 s4mul(scalar4 a, scalar4 b);
scalar4 s4div(scalar4 a, scalar4 b);
scalar4 s4load(scalar x);
scalar4 s4loadat(scalar4 a, scalar x, int i);
scalar4 s4abs(scalar4 a);
scalar4 s4sqrt(scalar4 a);
scalar4 v3x4dot(vec3x4 v, vec3x4 w);

vec3x4 v3x4load(vec3 v);
vec3x4 v3x4loadat(vec3x4 vv, vec3 v, int i);
vec3x4 v3x4sub(vec3x4 v, vec3x4 w);
vec3x4 v3x4mul(vec3x4 v, vec3x4 w);

scalar s4get(scalar4 a, int i);
vec3 v3x4get(vec3x4 v, int i);

#endif
