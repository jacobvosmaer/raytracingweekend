#include "shared.h"

#include <math.h>
#include <stdlib.h>

double randomdouble(void) { return rand() / (RAND_MAX + 1.0); }

struct interval interval(double min, double max) {
  struct interval iv;
  iv.min = min;
  iv.max = max;
  return iv;
}

int intervalsurrounds(struct interval iv, double x) {
  return x > iv.min && x < iv.max;
}

double intervalclamp(struct interval iv, double x) {
  if (x < iv.min)
    return iv.min;
  else if (x > iv.max)
    return iv.max;
  else
    return x;
}

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

vec3 v3scale(vec3 v, double c) {
  v.x *= c;
  v.y *= c;
  v.z *= c;
  return v;
}

double v3length(vec3 v) { return sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }
double v3dot(vec3 v, vec3 w) { return v.x * w.x + v.y * w.y + v.z * w.z; }
vec3 v3unit(vec3 v) { return v3scale(v, 1.0 / v3length(v)); }

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
  while (v3dot(v, v) >= 1);
  return v3unit(v);
}

vec3 v3clamp(vec3 v, struct interval iv) {
  return v3(intervalclamp(iv, v.x), intervalclamp(iv, v.y),
            intervalclamp(iv, v.z));
}

int v3nearzero(vec3 v) {
  double s = 1e-8;
  return fabs(v.x) < s && fabs(v.y) < s && fabs(v.z) < s;
}

vec3 rayat(ray r, double t) { return v3add(r.orig, v3scale(r.dir, t)); }

ray rayfromto(vec3 from, vec3 to) {
  ray r;
  r.orig = from;
  r.dir = v3sub(to, from);
  return r;
}

struct sphere sphere(vec3 center, double radius, material mat) {
  struct sphere sp;
  sp.center = center;
  sp.radius = radius;
  sp.mat = mat;
  return sp;
}

void spherelistadd(spherelist *sl, struct sphere sp) {
  if (sl->n == sl->max) {
    sl->max = sl->max ? 2 * sl->max : 1;
    assert(sl->spheres = realloc(sl->spheres, sl->max * sizeof(*sl->spheres)));
  }
  sl->spheres[sl->n++] = sp;
}

int lambertianscatter(ray in, hitrecord *rec, vec3 *attenuation, ray *scattered,
                      void *userdata) {
  vec3 scatterdirection = v3add(rec->normal, v3randomunit());
  if (v3nearzero(scatterdirection))
    scatterdirection = rec->normal;
  scattered->orig = rec->p;
  scattered->dir = scatterdirection;
  *attenuation = *(vec3 *)userdata;
  return 1;
}

material lambertian(vec3 albedo) {
  material mat;
  mat.scatter = lambertianscatter;
  assert(mat.userdata = malloc(sizeof(vec3)));
  *((vec3 *)mat.userdata) = albedo;
  return mat;
}
