#ifndef VEC3_H
#define VEC3_H

#if defined(__ARM_NEON)

#include <arm_neon.h>
typedef float32x4_t vec3;

#else

typedef struct {
  float x, y, z;
} vec3;

#endif

float v3x(vec3 v);
float v3y(vec3 v);
float v3z(vec3 v);

vec3 v3(float x, float y, float z);
vec3 v3add(vec3 v, vec3 w);
vec3 v3sub(vec3 v, vec3 w);
vec3 v3neg(vec3 v);
vec3 v3mul(vec3 v, vec3 w);
vec3 v3scale(vec3 v, float c);
float v3dot(vec3 v, vec3 w);
float v3length(vec3 v);
vec3 v3unit(vec3 v);
vec3 v3cross(vec3 v, vec3 w);
vec3 v3random(void);
vec3 v3randominterval(float min, float max);
vec3 v3randomunit(void);
vec3 v3randominunitdisk(void);
int v3nearzero(vec3 v);

float randomfloat(void);

#endif
