#ifndef VEC3_H
#define VEC3_H

#if defined(__x86_64__) && !defined(USESSE)
#define USESSE 0
#endif

#if defined(__ARM_NEON) && !defined(USENEON)
#define USENEON 0
#endif

#if USENEON

#include <arm_neon.h>
typedef float32x4_t vec3;
typedef float scalar;

#elif USESSE

#include <immintrin.h>
typedef __m128 vec3;
typedef float scalar;

#else

typedef float scalar;
typedef struct {
  scalar x, y, z;
} vec3;

#endif

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

#endif
