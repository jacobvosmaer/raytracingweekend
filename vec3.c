#include "vec3.h"

#include <math.h>
#include <stdlib.h>

static vec3 zero;

#if defined(__ARM_NEON)

float v3x(vec3 v) { return v[0]; }
float v3y(vec3 v) { return v[1]; }
float v3z(vec3 v) { return v[2]; }

vec3 v3(float x, float y, float z) {
  float32_t ar[4];
  ar[0] = x;
  ar[1] = y;
  ar[2] = z;
  ar[3] = 0;
  return vld1q_f32(ar);
}

vec3 v3add(vec3 v, vec3 w) { return vaddq_f32(v, w); }
vec3 v3sub(vec3 v, vec3 w) { return vsubq_f32(v, w); }
vec3 v3mul(vec3 v, vec3 w) { return vmulq_f32(v, w); }
vec3 v3scale(vec3 v, float c) { return vmulq_n_f32(v, c); }
float v3dot(vec3 v, vec3 w) { return vaddvq_f32(vmulq_f32(v, w)); }

#elif defined(__x86_64__)

float v3x(vec3 v) { return v[0]; }
float v3y(vec3 v) { return v[1]; }
float v3z(vec3 v) { return v[2]; }

vec3 v3(float x, float y, float z) {
  float ar[4];
  ar[0] = x;
  ar[1] = y;
  ar[2] = z;
  ar[3] = 0;
  return _mm_load_ps(ar);
}

vec3 v3add(vec3 v, vec3 w) { return _mm_add_ps(v, w); }
vec3 v3sub(vec3 v, vec3 w) { return _mm_sub_ps(v, w); }
vec3 v3mul(vec3 v, vec3 w) { return _mm_mul_ps(v, w); }
vec3 v3scale(vec3 v, float c) { return v3mul(v, _mm_load_ps1(&c)); }
float v3dot(vec3 v, vec3 w) {
  v = v3mul(v, w);
  return v[0] + v[1] + v[2] + v[3];
}

#else

float v3x(vec3 v) { return v.x; }
float v3y(vec3 v) { return v.y; }
float v3z(vec3 v) { return v.z; }

vec3 v3(float x, float y, float z) {
  vec3 v;
  v.x = x;
  v.y = y;
  v.z = z;
  return v;
}

vec3 v3add(vec3 v, vec3 w) {
  v.x += w.x;
  v.y += w.y;
  v.z += w.z;
  return v;
}

vec3 v3sub(vec3 v, vec3 w) {
  v.x -= w.x;
  v.y -= w.y;
  v.z -= w.z;
  return v;
}

vec3 v3mul(vec3 v, vec3 w) {
  v.x *= w.x;
  v.y *= w.y;
  v.z *= w.z;
  return v;
}

vec3 v3scale(vec3 v, float c) {
  v.x *= c;
  v.y *= c;
  v.z *= c;
  return v;
}

float v3dot(vec3 v, vec3 w) { return v.x * w.x + v.y * w.y + v.z * w.z; }

#endif

vec3 v3neg(vec3 v) { return v3sub(zero, v); }

float v3length(vec3 v) { return sqrtf(v3dot(v, v)); }
vec3 v3unit(vec3 v) { return v3scale(v, 1.0 / v3length(v)); }
vec3 v3cross(vec3 v, vec3 w) {
  return v3(v3y(v) * v3z(w) - v3z(v) * v3y(w),
            v3z(v) * v3x(w) - v3x(v) * v3z(w),
            v3x(v) * v3y(w) - v3y(v) * v3x(w));
}

vec3 v3random(void) { return v3(randomfloat(), randomfloat(), randomfloat()); }

vec3 v3randominterval(float min, float max) {
  return v3add(v3(min, min, min), v3scale(v3random(), max - min));
}

vec3 v3randomunit(void) {
  while (1) {
    vec3 v = v3randominterval(-1, 1);
    if (v3dot(v, v) < 1)
      return v3unit(v);
  }
}

vec3 v3randominunitdisk(void) {
  while (1) {
    vec3 v = v3(-1.0 + 2.0 * randomfloat(), -1.0 + 2.0 * randomfloat(), 0);
    if (v3dot(v, v) < 1)
      return v;
  }
}
