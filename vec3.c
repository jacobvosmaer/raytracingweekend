#include "vec3.h"
#include "random.h"

#include <math.h>
#include <stdlib.h>

static vec3 zero;

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

vec3x4 v3x4load(vec3 v) {
  vec3x4 vv;
  vv.x = s4load(v3x(v));
  vv.y = s4load(v3y(v));
  vv.z = s4load(v3z(v));
  return vv;
}

vec3x4 v3x4sub(vec3x4 v, vec3x4 w) {
  v.x = s4sub(v.x, w.x);
  v.y = s4sub(v.y, w.y);
  v.z = s4sub(v.z, w.z);
  return v;
}

vec3x4 v3x4mul(vec3x4 v, vec3x4 w) {
  v.x = s4mul(v.x, w.x);
  v.y = s4mul(v.y, w.y);
  v.z = s4mul(v.z, w.z);
  return v;
}

scalar4 v3x4dot(vec3x4 v, vec3x4 w) {
  v = v3x4mul(v, w);
  return s4add(s4add(v.x, v.y), v.z);
}

vec3 v3x4get(vec3x4 v, int i) {
  return v3(s4get(v.x, i), s4get(v.y, i), s4get(v.z, i));
}

vec3x4 v3x4loadat(vec3x4 vv, vec3 v, int i) {
  vv.x = s4loadat(vv.x, v3x(v), i);
  vv.y = s4loadat(vv.y, v3y(v), i);
  vv.z = s4loadat(vv.z, v3z(v), i);
  return vv;
}
