#include "vec3.h"
#include "random.h"

#include <math.h>
#include <stdlib.h>

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
vec3 v3neg(vec3 v) { return v3(-v.x, -v.y, -v.z); }
scalar v3length(vec3 v) { return sqrtf(v3dot(v, v)); }
vec3 v3unit(vec3 v) { return v3scale(v, 1.0 / v3length(v)); }

vec3 v3cross(vec3 v, vec3 w) {
  return v3(v.y * w.z - v.z * w.y, v.z * w.x - v.x * w.z,
            v.x * w.y - v.y * w.x);
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

vec3x8 v3x8load(vec3 v) {
  vec3x8 vv;
  vv.x[0] = vv.x[1] = s4load(v.x);
  vv.y[0] = vv.y[1] = s4load(v.y);
  vv.z[0] = vv.z[1] = s4load(v.z);
  return vv;
}

vec3x8 v3x8sub(vec3x8 v, vec3x8 w) {
  v.x[0] = s4sub(v.x[0], w.x[0]);
  v.x[1] = s4sub(v.x[1], w.x[1]);
  v.y[0] = s4sub(v.y[0], w.y[0]);
  v.y[1] = s4sub(v.y[1], w.y[1]);
  v.z[0] = s4sub(v.z[0], w.z[0]);
  v.z[1] = s4sub(v.z[1], w.z[1]);
  return v;
}

scalar4 v3x8dot(vec3x8 v, vec3x8 w, int i) {
  return s4mulacc(s4mulacc(s4mul(v.x[i], w.x[i]), v.y[i], w.y[i]), v.z[i],
                  w.z[i]);
}

vec3 v3x8get(vec3x8 v, int i) {
  return v3(s4get(v.x[i / 4], i % 4), s4get(v.y[i / 4], i % 4),
            s4get(v.z[i / 4], i % 4));
}

vec3x8 v3x8loadat(vec3x8 vv, vec3 v, int i) {
  assert(i >= 0);
  assert(i < 8);
  vv.x[i / 4] = s4loadat(vv.x[i / 4], v.x, i % 4);
  vv.y[i / 4] = s4loadat(vv.y[i / 4], v.y, i % 4);
  vv.z[i / 4] = s4loadat(vv.z[i / 4], v.z, i % 4);
  return vv;
}
