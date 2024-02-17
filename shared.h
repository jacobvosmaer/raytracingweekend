#ifndef VEC3_H
#define VEC3_H

#define assert(x)                                                              \
  if (!(x))                                                                    \
  __builtin_trap()

double randomdouble(void);

typedef struct {
  double x, y, z;
} vec3;
vec3 v3(double x, double y, double z);
vec3 v3add(vec3 v, vec3 w);
vec3 v3sub(vec3 v, vec3 w);
vec3 v3neg(vec3 v);
vec3 v3scale(vec3 v, double c);
double v3length(vec3 v);
double v3dot(vec3 v, vec3 w);
vec3 v3unit(vec3 v);
vec3 v3random(void);
vec3 v3randominterval(double min, double max);
vec3 v3randomunit(void);

typedef struct {
  vec3 orig, dir;
} ray;
vec3 rayat(ray r, double t);
ray rayfromto(vec3 from, vec3 to);

struct sphere {
  vec3 center;
  double radius;
};
struct sphere sphere(vec3 center, double radius);

typedef struct {
  struct sphere *spheres;
  int n, max;
} spherelist;
void spherelistadd(spherelist *sl, struct sphere sp);

#endif
