#include "shared.h"

#include <math.h>
#include <stdlib.h>

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

vec3 v3scale(vec3 v, double c) {
  v.x *= c;
  v.y *= c;
  v.z *= c;
  return v;
}

double v3length(vec3 v) { return sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }
double v3dot(vec3 v, vec3 w) { return v.x * w.x + v.y * w.y + v.z * w.z; }
vec3 v3unit(vec3 v) { return v3scale(v, 1.0 / v3length(v)); }

struct sphere sphere(vec3 center, double radius) {
  struct sphere sp;
  sp.center = center;
  sp.radius = radius;
  return sp;
}

void spherelistadd(spherelist *sl, struct sphere sp) {
  if (sl->n == sl->max) {
    sl->max = sl->max ? 2 * sl->max : 1;
    assert(sl->spheres = realloc(sl->spheres, sl->max * sizeof(*sl->spheres)));
  }
  sl->spheres[sl->n++] = sp;
}
