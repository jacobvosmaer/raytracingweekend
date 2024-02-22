#include "vec3.h"
#include "random.h"

#include <math.h>
#include <stdlib.h>

static vec3 zero;

#if USENEON

scalar v3x(vec3 v) { return v[0]; }
scalar v3y(vec3 v) { return v[1]; }
scalar v3z(vec3 v) { return v[2]; }

vec3 v3(scalar x, scalar y, scalar z) {
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
vec3 v3scale(vec3 v, scalar c) { return vmulq_n_f32(v, c); }
scalar v3dot(vec3 v, vec3 w) { return vaddvq_f32(vmulq_f32(v, w)); }

#elif USESSE

scalar v3x(vec3 v) { return v[0]; }
scalar v3y(vec3 v) { return v[1]; }
scalar v3z(vec3 v) { return v[2]; }

vec3 v3(scalar x, scalar y, scalar z) {
  scalar ar[4];
  ar[0] = x;
  ar[1] = y;
  ar[2] = z;
  ar[3] = 0;
  return _mm_load_ps(ar);
}

vec3 v3add(vec3 v, vec3 w) { return _mm_add_ps(v, w); }
vec3 v3sub(vec3 v, vec3 w) { return _mm_sub_ps(v, w); }
vec3 v3mul(vec3 v, vec3 w) { return _mm_mul_ps(v, w); }
vec3 v3scale(vec3 v, scalar c) { return _mm_mul_ps(v, _mm_load_ps1(&c)); }
scalar v3dot(vec3 v, vec3 w) {
  v = _mm_mul_ps(v, w);
  v = _mm_hadd_ps(v, v);
  v = _mm_hadd_ps(v, v);
  return v[0];
}

#else

scalar v3x(vec3 v) { return v.x; }
scalar v3y(vec3 v) { return v.y; }
scalar v3z(vec3 v) { return v.z; }

vec3 v3(scalar x, scalar y, scalar z) {
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

vec3 v3scale(vec3 v, scalar c) {
  v.x *= c;
  v.y *= c;
  v.z *= c;
  return v;
}

scalar v3dot(vec3 v, vec3 w) { return v.x * w.x + v.y * w.y + v.z * w.z; }

#endif

vec3 v3neg(vec3 v) { return v3sub(zero, v); }

scalar v3length(vec3 v) { return sqrtf(v3dot(v, v)); }
vec3 v3unit(vec3 v) { return v3scale(v, 1.0 / v3length(v)); }
vec3 v3cross(vec3 v, vec3 w) {
  return v3(v3y(v) * v3z(w) - v3z(v) * v3y(w),
            v3z(v) * v3x(w) - v3x(v) * v3z(w),
            v3x(v) * v3y(w) - v3y(v) * v3x(w));
}

vec3 v3random(void) {
  return v3(randomscalar(), randomscalar(), randomscalar());
}

vec3 v3randominterval(scalar min, scalar max) {
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
    vec3 v = v3(-1.0 + 2.0 * randomscalar(), -1.0 + 2.0 * randomscalar(), 0);
    if (v3dot(v, v) < 1)
      return v;
  }
}
