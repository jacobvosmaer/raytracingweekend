#include "vec3.h"

#include <math.h>
#include <stdlib.h>

static double randomdouble(void) { return rand() / (RAND_MAX + 1.0); }

vec3 v3(double x, double y, double z) {
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

vec3 v3neg(vec3 v) {
  vec3 zero = {0};
  return v3sub(zero, v);
}

vec3 v3mul(vec3 v, vec3 w) {
  v.x *= w.x;
  v.y *= w.y;
  v.z *= w.z;
  return v;
}

vec3 v3scale(vec3 v, double c) {
  v.x *= c;
  v.y *= c;
  v.z *= c;
  return v;
}

double v3dot(vec3 v, vec3 w) { return v.x * w.x + v.y * w.y + v.z * w.z; }
double v3length(vec3 v) { return sqrt(v3dot(v, v)); }
vec3 v3unit(vec3 v) { return v3scale(v, 1.0 / v3length(v)); }

vec3 v3cross(vec3 v, vec3 w) {
  return v3(v.y * w.z - v.z * w.y, v.z * w.x - v.x * w.z,
            v.x * w.y - v.y * w.x);
}

vec3 v3random(void) {
  return v3(randomdouble(), randomdouble(), randomdouble());
}

vec3 v3randominterval(double min, double max) {
  return v3add(v3(min, min, min), v3scale(v3random(), max - min));
}

vec3 v3randomunit(void) {
  vec3 v;
  do
    v = v3randominterval(-1, 1);
  while (v3dot(v, v) > 1);
  return v3unit(v);
}

vec3 v3randominunitdisk(void) {
  while (1) {
    vec3 v = v3randominterval(-1, 1);
    v.z = 0;
    if (v3dot(v, v) < 1)
      return v;
  }
}
