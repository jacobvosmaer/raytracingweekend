#include <math.h>
#include <stdio.h>
#include <stdlib.h>

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

enum material { LAMBERTIAN, METAL, DIELECTRIC };

typedef struct {
  enum material type;
  union {
    struct lambertian {
      vec3 albedo;
    } lambertian;
    struct metal {
      vec3 albedo;
      double fuzz;
    } metal;
    struct dielectric {
      double ir;
    } dielectric;
  } data;
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

typedef struct {
  vec3 p, normal;
  double t;
  int frontface;
  material mat;
} hitrecord;

double randomdouble(void) { return rand() / (RAND_MAX + 1.0); }

material lambertian(vec3 albedo) {
  material mat;
  mat.type = LAMBERTIAN;
  mat.data.lambertian.albedo = albedo;
  return mat;
}

material metal(vec3 albedo, double fuzz) {
  material mat;
  mat.type = METAL;
  mat.data.metal.albedo = albedo;
  mat.data.metal.fuzz = fuzz > 1 ? 1 : fuzz;
  return mat;
}

material dielectric(double ir) {
  material mat;
  mat.type = DIELECTRIC;
  mat.data.dielectric.ir = ir;
  return mat;
}

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

void writecolor(FILE *out, vec3 color, int nsamples) {
  color = v3scale(color, 1.0 / nsamples);
  /* Use sqrt as gamma correction */
  color = v3clamp(v3(sqrt(color.x), sqrt(color.y), sqrt(color.z)),
                  interval(0, 0.999));
  fprintf(out, "%d %d %d\n", (int)(256 * color.x), (int)(256 * color.y),
          (int)(256 * color.z));
}

/* outwardnormal must be unit vector */
void hitrecordsetnormal(hitrecord *rec, ray r, vec3 outwardnormal) {
  rec->frontface = v3dot(r.dir, outwardnormal) < 0;
  rec->normal = rec->frontface ? outwardnormal : v3neg(outwardnormal);
}

int spherehit(struct sphere sp, ray r, struct interval t, hitrecord *rec) {
  vec3 oc = v3sub(r.orig, sp.center);
  double a = v3dot(r.dir, r.dir);
  double halfb = v3dot(oc, r.dir);
  double c = v3dot(oc, oc) - sp.radius * sp.radius;
  double discriminant = halfb * halfb - a * c;
  double sqrtd, root;

  if (discriminant < 0)
    return 0;
  sqrtd = sqrt(discriminant);
  root = (-halfb - sqrtd) / a;
  if (!intervalsurrounds(t, root)) {
    root = (-halfb + sqrtd) / a;
    if (!intervalsurrounds(t, root))
      return 0;
  }

  rec->t = root;
  rec->p = rayat(r, rec->t);
  hitrecordsetnormal(rec, r,
                     v3scale(v3sub(rec->p, sp.center), 1.0 / sp.radius));
  rec->mat = sp.mat;
  return 1;
}

int spherelisthit(spherelist *sl, ray r, struct interval t, hitrecord *rec) {
  int i, nhit = 0;
  double closest = t.max;

  for (i = 0; i < sl->n; i++) {
    if (spherehit(sl->spheres[i], r, interval(t.min, closest), rec)) {
      nhit++;
      closest = rec->t;
    }
  }
  return !!nhit;
}

vec3 randomonhemisphere(vec3 normal) {
  vec3 v = v3randomunit();
  if (v3dot(v, normal) > 0.0)
    return v;
  else
    return v3neg(v);
}

vec3 reflect(vec3 v, vec3 n) { return v3sub(v, v3scale(n, 2 * v3dot(v, n))); }

vec3 refract(vec3 uv, vec3 n, double etaioveretat) {
  double costheta = fmin(v3dot(v3neg(uv), n), 1);
  vec3 routperp = v3scale(v3add(uv, v3scale(n, costheta)), etaioveretat),
       routparallel = v3scale(n, -sqrt(fabs(1.0 - v3dot(routperp, routperp))));
  return v3add(routperp, routparallel);
}

int scatter(material mat, ray in, hitrecord *rec, vec3 *attenuation,
            ray *scattered) {
  if (mat.type == LAMBERTIAN) {
    struct lambertian data = mat.data.lambertian;
    vec3 scatterdirection = v3add(rec->normal, v3randomunit());
    if (v3nearzero(scatterdirection))
      scatterdirection = rec->normal;
    scattered->orig = rec->p;
    scattered->dir = scatterdirection;
    *attenuation = data.albedo;
    return 1;
  } else if (mat.type == METAL) {
    struct metal data = mat.data.metal;
    vec3 reflected = reflect(v3unit(in.dir), rec->normal);
    scattered->orig = rec->p;
    scattered->dir = v3add(reflected, v3scale(v3randomunit(), data.fuzz));
    *attenuation = data.albedo;
    return 1;
  } else if (mat.type == DIELECTRIC) {
    struct dielectric data = mat.data.dielectric;
    vec3 transparent = {1, 1, 1};
    double refractionratio = rec->frontface ? 1.0 / data.ir : data.ir;
    vec3 unitdirection = v3unit(in.dir);
    double costheta = fmin(v3dot(v3neg(unitdirection), rec->normal), 1.0),
           sintheta = sqrt(1.0 - costheta * costheta);
    *attenuation = transparent;
    scattered->orig = rec->p;
    scattered->dir = refractionratio * sintheta > 1.0
                         ? reflect(unitdirection, rec->normal)
                         : refract(unitdirection, rec->normal, refractionratio);
    return 1;
  } else {
    return 0;
  }
}

vec3 raycolor(ray r, int depth, spherelist *world) {
  vec3 black = {0};
  hitrecord rec;

  if (depth <= 0)
    return black;

  if (spherelisthit(world, r, interval(0.001, INFINITY), &rec)) {
    ray scattered;
    vec3 attenuation;
    if (scatter(rec.mat, r, &rec, &attenuation, &scattered))
      return v3mul(attenuation, raycolor(scattered, depth - 1, world));
    return black;
  } else {
    vec3 dir = v3unit(r.dir);
    double a = 0.5 * (dir.y + 1.0);
    return v3add(v3scale(v3(1, 1, 1), 1.0 - a), v3scale(v3(0.5, 0.7, 1), a));
  }
}

typedef struct {
  double aspectratio;
  int imagewidth, samplesperpixel, maxdepth;

  /* derived values */
  int imageheight;
  vec3 center, pixeldu, pixeldv, pixel00loc;
} camera;

#define CAMERADEFAULT                                                          \
  { 1.0, 100, 10, 10 }

vec3 pixelsamplesquare(camera *c) {
  double px = -0.5 + randomdouble(), py = -0.5 + randomdouble();
  return v3add(v3scale(c->pixeldu, px), v3scale(c->pixeldv, py));
}

ray getray(camera *c, int i, int j) {
  vec3 pixelcenter = v3add(v3add(c->pixel00loc, v3scale(c->pixeldu, i)),
                           v3scale(c->pixeldv, j)),
       pixelsample = v3add(pixelcenter, pixelsamplesquare(c));
  return rayfromto(c->center, pixelsample);
}

void camerainitialize(camera *c) {
  double focallength = 1, viewportheight = 2, viewportwidth;
  vec3 viewportu, viewportv, viewportupperleft;

  c->imageheight = c->imagewidth / c->aspectratio;
  if (c->imageheight < 1)
    c->imageheight = 1;
  viewportwidth = viewportheight * ((double)c->imagewidth / c->imageheight);
  viewportu = v3(viewportwidth, 0, 0);
  viewportv = v3(0, -viewportheight, 0);
  c->pixeldu = v3scale(viewportu, 1.0 / c->imagewidth);
  c->pixeldv = v3scale(viewportv, 1.0 / c->imageheight);

  viewportupperleft = v3add(
      v3add(v3add(c->center, v3(0, 0, -focallength)), v3scale(viewportu, -0.5)),
      v3scale(viewportv, -0.5));
  c->pixel00loc =
      v3add(viewportupperleft, v3scale(v3add(c->pixeldu, c->pixeldv), 0.5));
}

void camerarender(camera *c, spherelist *world) {
  int i, j;

  camerainitialize(c);
  printf("P3\n%d %d\n255\n", c->imagewidth, c->imageheight);

  for (j = 0; j < c->imageheight; j++) {
    for (i = 0; i < c->imagewidth; i++) {
      vec3 pixelcolor = {0};
      int k;
      for (k = 0; k < c->samplesperpixel; k++)
        pixelcolor =
            v3add(pixelcolor, raycolor(getray(c, i, j), c->maxdepth, world));
      writecolor(stdout, pixelcolor, c->samplesperpixel);
    }
  }
}

int main(void) {
  camera cam = CAMERADEFAULT;
  spherelist world = {0};
  material matground = lambertian(v3(0.8, 0.8, 0)),
           matcenter = lambertian(v3(0.1, 0.2, 0.5)), matleft = dielectric(1.5),
           matright = metal(v3(0.8, 0.6, 0.2), 0);

  spherelistadd(&world, sphere(v3(0, -100.5, -1), 100, matground));
  spherelistadd(&world, sphere(v3(0, 0, -1), 0.5, matcenter));
  spherelistadd(&world, sphere(v3(-1, 0, -1), 0.5, matleft));
  spherelistadd(&world, sphere(v3(1, 0, -1), 0.5, matright));

  cam.aspectratio = 16.0 / 9.0;
  cam.imagewidth = 400;
  cam.samplesperpixel = 100;
  cam.maxdepth = 50;
  camerarender(&cam, &world);

  return 0;
}
