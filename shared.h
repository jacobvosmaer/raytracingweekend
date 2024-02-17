#ifndef SHARED_H
#define SHARED_H

#define assert(x)                                                              \
  if (!(x))                                                                    \
  __builtin_trap()

struct interval {
  double min, max;
};

typedef struct {
  double x, y, z;
} vec3;

typedef struct {
  vec3 orig, dir;
} ray;

typedef struct hitrecord hitrecord;

typedef struct material {
  int (*scatter)(ray in, hitrecord *rec, vec3 *attenuation, ray *scattered,
                 void *userdata);
  void *userdata;
} material;

struct sphere {
  vec3 center;
  double radius;
  material mat;
};

typedef struct {
  struct sphere *spheres;
  int n, max;
} spherelist;

struct hitrecord {
  vec3 p, normal;
  double t;
  int frontface;
  material mat;
};

double randomdouble(void);

struct interval interval(double min, double max);
int intervalsurrounds(struct interval iv, double x);
double intervalclamp(struct interval iv, double x);

vec3 v3(double x, double y, double z);
vec3 v3add(vec3 v, vec3 w);
vec3 v3sub(vec3 v, vec3 w);
vec3 v3neg(vec3 v);
vec3 v3mul(vec3 v, vec3 w);
vec3 v3scale(vec3 v, double c);
double v3length(vec3 v);
double v3dot(vec3 v, vec3 w);
vec3 v3unit(vec3 v);
vec3 v3random(void);
vec3 v3randominterval(double min, double max);
vec3 v3randomunit(void);
vec3 v3clamp(vec3 v, struct interval iv);
int v3nearzero(vec3 v);

vec3 rayat(ray r, double t);
ray rayfromto(vec3 from, vec3 to);

struct sphere sphere(vec3 center, double radius, material mat);

void spherelistadd(spherelist *sl, struct sphere sp);

material lambertian(vec3 albedo);
material metal(vec3 albedo);

#endif
