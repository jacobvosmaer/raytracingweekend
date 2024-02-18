#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef NTHREADS
#define NTHREADS 8
#endif

#include "vec3.h"

#define assert(x)                                                              \
  if (!(x))                                                                    \
  __builtin_trap()

struct interval {
  float min, max;
};

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
      float fuzz;
    } metal;
    struct dielectric {
      float ir;
    } dielectric;
  } data;
} material;

struct sphere {
  vec3 center;
  float radius;
  material mat;
};

typedef struct {
  struct sphere *spheres;
  int n, max;
} spherelist;

typedef struct {
  vec3 p, normal;
  float t;
  int frontface;
  material mat;
} hitrecord;

float randomfloat(void) { return random() / ((float)(1L << 31)); }

float pi = 3.1415926536;

float degtorad(float deg) { return pi * deg / 180.0; }

material lambertian(vec3 albedo) {
  material mat;
  mat.type = LAMBERTIAN;
  mat.data.lambertian.albedo = albedo;
  return mat;
}

material metal(vec3 albedo, float fuzz) {
  material mat;
  mat.type = METAL;
  mat.data.metal.albedo = albedo;
  mat.data.metal.fuzz = fuzz > 1 ? 1 : fuzz;
  return mat;
}

material dielectric(float ir) {
  material mat;
  mat.type = DIELECTRIC;
  mat.data.dielectric.ir = ir;
  return mat;
}

struct interval interval(float min, float max) {
  struct interval iv;
  iv.min = min;
  iv.max = max;
  return iv;
}

int intervalsurrounds(struct interval iv, float x) {
  return x > iv.min && x < iv.max;
}

float intervalclamp(struct interval iv, float x) {
  if (x < iv.min)
    return iv.min;
  else if (x > iv.max)
    return iv.max;
  else
    return x;
}

int v3nearzero(vec3 v) {
  float s = 1e-8;
  return fabsf(v3x(v)) < s && fabsf(v3y(v)) < s && fabsf(v3z(v)) < s;
}

vec3 rayat(ray r, float t) { return v3add(r.orig, v3scale(r.dir, t)); }

ray rayfromto(vec3 from, vec3 to) {
  ray r;
  r.orig = from;
  r.dir = v3sub(to, from);
  return r;
}

struct sphere sphere(vec3 center, float radius, material mat) {
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
  float r, g, b;
  struct interval intensity = interval(0, 0.999);
  color = v3scale(color, 1.0 / nsamples);
  /* Use sqrt as gamma correction */
  r = intervalclamp(intensity, sqrtf(v3x(color)));
  g = intervalclamp(intensity, sqrtf(v3y(color)));
  b = intervalclamp(intensity, sqrtf(v3z(color)));
  fprintf(out, "%d %d %d\n", (int)(256 * r), (int)(256 * g), (int)(256 * b));
}

/* outwardnormal must be unit vector */
void hitrecordsetnormal(hitrecord *rec, ray r, vec3 outwardnormal) {
  rec->frontface = v3dot(r.dir, outwardnormal) < 0;
  rec->normal = rec->frontface ? outwardnormal : v3neg(outwardnormal);
}

int spherehit(struct sphere sp, ray r, struct interval t, hitrecord *rec) {
  vec3 oc = v3sub(r.orig, sp.center);
  float a = v3dot(r.dir, r.dir);
  float halfb = v3dot(oc, r.dir);
  float c = v3dot(oc, oc) - sp.radius * sp.radius;
  float discriminant = halfb * halfb - a * c;
  float sqrtd, root;

  if (discriminant < 0)
    return 0; /* The ray does not hit the sphere */

  /* The ray hits the sphere in 1 or 2 points (roots). Do any of the roots lie
   * in the interval? */
  sqrtd = sqrtf(discriminant);
  root = (-halfb - sqrtd) / a; /* Prefer the nearest root. */
  if (!intervalsurrounds(t, root)) {
    root = (-halfb + sqrtd) / a;
    if (!intervalsurrounds(t, root))
      return 0;
  }

  /* Root is the nearest intersection of the ray and the sphere */
  rec->t = root;
  rec->p = rayat(r, rec->t);
  hitrecordsetnormal(rec, r,
                     v3scale(v3sub(rec->p, sp.center), 1.0 / sp.radius));
  rec->mat = sp.mat;
  return 1;
}

int spherelisthit(spherelist *sl, ray r, struct interval t, hitrecord *rec) {
  int i, hit = 0;
  float closest = t.max;

  for (i = 0; i < sl->n; i++) {
    if (spherehit(sl->spheres[i], r, interval(t.min, closest), rec)) {
      hit = 1;
      closest = rec->t;
    }
  }
  return hit;
}

vec3 reflect(vec3 v, vec3 n) { return v3sub(v, v3scale(n, 2 * v3dot(v, n))); }

vec3 refract(vec3 uv, vec3 n, float etaioveretat) {
  float costheta = fmin(v3dot(v3neg(uv), n), 1);
  vec3 routperp = v3scale(v3add(uv, v3scale(n, costheta)), etaioveretat),
       routparallel = v3scale(n, -sqrtf(fabs(1.0 - v3dot(routperp, routperp))));
  return v3add(routperp, routparallel);
}

float reflectance(float cosine, float refidx) {
  float r0 = (1.0 - refidx) / (1.0 + refidx);
  r0 *= r0;
  return r0 + (1.0 - r0) * pow(1.0 - cosine, 5);
}

/* Scatter factors out the material-specific behavior of the ray-tracing
 * function raycolor below. If scatter returns 0 the in ray has been fully
 * absorbed. If it returns a non-zero value then attenuation and scattered
 * describe the outbound ray. */
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
    float refractionratio = rec->frontface ? 1.0 / data.ir : data.ir;
    vec3 unitdirection = v3unit(in.dir);
    float costheta = fmin(v3dot(v3neg(unitdirection), rec->normal), 1.0),
          sintheta = sqrtf(1.0 - costheta * costheta);
    scattered->orig = rec->p;
    scattered->dir =
        refractionratio * sintheta > 1.0 ||
                reflectance(costheta, refractionratio) > randomfloat()
            ? reflect(unitdirection, rec->normal)
            : refract(unitdirection, rec->normal, refractionratio);
    *attenuation = v3(1, 1, 1);
    return 1;
  } else {
    return 0;
  }
}

/* Raycolor recursively traces a single ray through the scene to compute its
 * color. When a ray hits a dielectric material it randomly either reflects or
 * refracts, i.e. the ray does not split. Because raycolor gets called more than
 * once per pixel we still get the effect that dielectrics both reflect _and_
 * refract. */
vec3 raycolor(ray r, int depth, spherelist *world) {
  vec3 black = {0};
  hitrecord rec;

  if (depth <= 0)
    return black;

  if (spherelisthit(world, r, interval(0.001, INFINITY), &rec)) {
    /* ray has hit an object */
    ray scattered;
    vec3 attenuation;
    if (scatter(rec.mat, r, &rec, &attenuation, &scattered))
      return v3mul(attenuation, raycolor(scattered, depth - 1, world));
    return black;
  } else {
    /* ray has hit the sky */
    vec3 dir = v3unit(r.dir);
    float a = 0.5 * (v3y(dir) + 1.0);
    return v3add(v3scale(v3(1, 1, 1), 1.0 - a), v3scale(v3(0.5, 0.7, 1), a));
  }
}

typedef struct {
  float aspectratio;
  int imagewidth, samplesperpixel, maxdepth;
  float vfov;
  vec3 lookfrom, lookat, vup;
  float defocusangle, focusdist;

  /* derived values */
  int imageheight;
  vec3 center, pixeldu, pixeldv, pixel00loc, u, v, w, defocusdisku,
      defocusdiskv;

  ray *rays;
  int nrays;
  pthread_mutex_t raymutex;
  pthread_cond_t raycond;

  vec3 *colors;
  int ncolors;
  pthread_mutex_t colormutex;
  pthread_cond_t colorcond;
} camera;

#define CAMERADEFAULT                                                          \
  { 1, 100, 10, 10, 90, {0, 0, -1}, {0, 0, 0}, {0, 1, 0}, 0, 10 }

vec3 pixelsamplesquare(camera *c) {
  float px = -0.5 + randomfloat(), py = -0.5 + randomfloat();
  return v3add(v3scale(c->pixeldu, px), v3scale(c->pixeldv, py));
}

vec3 defocusdisksample(camera *c) {
  vec3 p = v3randominunitdisk();
  return v3add(c->center, v3add(v3scale(c->defocusdisku, v3x(p)),
                                v3scale(c->defocusdiskv, v3y(p))));
}

/* Getray returns a random ray near i, j. The randomness is for anti-aliasing.
 */
ray getray(camera *c, int i, int j) {
  vec3 pixelcenter = v3add(v3add(c->pixel00loc, v3scale(c->pixeldu, i)),
                           v3scale(c->pixeldv, j)),
       pixelsample = v3add(pixelcenter, pixelsamplesquare(c));
  return rayfromto(c->defocusangle <= 0 ? c->center : defocusdisksample(c),
                   pixelsample);
}

void camerainitialize(camera *c) {
  float viewportheight, viewportwidth, h, defocusradius;
  vec3 viewportu, viewportv, viewportupperleft;

  c->imageheight = c->imagewidth / c->aspectratio;
  if (c->imageheight < 1)
    c->imageheight = 1;

  c->center = c->lookfrom;

  h = tan(degtorad(c->vfov) / 2);
  viewportheight = 2 * h * c->focusdist;
  viewportwidth = viewportheight * ((float)c->imagewidth / c->imageheight);

  c->w = v3unit(v3sub(c->lookfrom, c->lookat));
  c->u = v3unit(v3cross(c->vup, c->w));
  c->v = v3cross(c->w, c->u);

  viewportu = v3scale(c->u, viewportwidth);
  viewportv = v3scale(v3neg(c->v), viewportheight);
  c->pixeldu = v3scale(viewportu, 1.0 / c->imagewidth);
  c->pixeldv = v3scale(viewportv, 1.0 / c->imageheight);

  viewportupperleft = v3sub(v3sub(v3sub(c->center, v3scale(c->w, c->focusdist)),
                                  v3scale(viewportu, 0.5)),
                            v3scale(viewportv, 0.5));
  c->pixel00loc =
      v3add(viewportupperleft, v3scale(v3add(c->pixeldu, c->pixeldv), 0.5));

  defocusradius = c->focusdist * tan(degtorad(c->defocusangle / 2.0));
  c->defocusdisku = v3scale(c->u, defocusradius);
  c->defocusdiskv = v3scale(c->v, defocusradius);

  assert(!pthread_mutex_init(&c->raymutex, 0));
  assert(!pthread_cond_init(&c->raycond, 0));
  assert(c->rays = calloc(sizeof(*(c->rays)), c->samplesperpixel));

  assert(!pthread_mutex_init(&c->colormutex, 0));
  assert(!pthread_cond_init(&c->colorcond, 0));
  assert(c->colors = calloc(sizeof(*(c->colors)), c->samplesperpixel));
}

struct threaddata {
  camera *c;
  spherelist *world;
};

void *camerathread(void *userdata) {
  struct threaddata *td = userdata;
  camera *c = td->c;
  spherelist *world = td->world;
  ray *rays;
  int maxjobs;
  vec3 *colors;

  maxjobs = c->samplesperpixel / NTHREADS;
  assert(rays = calloc(sizeof(*rays), maxjobs));
  assert(colors = calloc(sizeof(*colors), maxjobs));

  while (1) {
    int i, njobs = 0;
    assert(!pthread_mutex_lock(&c->raymutex));
    while (!c->nrays)
      assert(!pthread_cond_wait(&c->raycond, &c->raymutex));
    while (njobs < maxjobs && c->nrays)
      rays[njobs++] = c->rays[--c->nrays];
    assert(!pthread_mutex_unlock(&c->raymutex));

    for (i = 0; i < njobs; i++)
      colors[i] = raycolor(rays[i], c->maxdepth, world);

    assert(!pthread_mutex_lock(&c->colormutex));
    for (i = 0; i < njobs; i++)
      c->colors[c->ncolors++] = colors[i];
    assert(!pthread_mutex_unlock(&c->colormutex));
    assert(!pthread_cond_broadcast(&c->colorcond));
  }
  return 0;
}

void camerarender(camera *c, spherelist *world) {
  int i, j, k;
  pthread_t threads[NTHREADS];
  struct threaddata td;

  camerainitialize(c);
  td.c = c;
  td.world = world;
  for (k = 0; k < NTHREADS; k++)
    assert(!pthread_create(threads + k, 0, camerathread, &td));

  printf("P3\n%d %d\n255\n", c->imagewidth, c->imageheight);

  for (j = 0; j < c->imageheight; j++) {
    for (i = 0; i < c->imagewidth; i++) {
      vec3 pixelcolor = {0};

      assert(!pthread_mutex_lock(&c->raymutex));
      assert(!c->nrays);
      for (k = 0; k < c->samplesperpixel; k++)
        c->rays[c->nrays++] = getray(c, i, j);
      assert(!pthread_mutex_unlock(&c->raymutex));
      assert(!pthread_cond_broadcast(&c->raycond));

      k = 0;
      while (k < c->samplesperpixel) {
        assert(!pthread_mutex_lock(&c->colormutex));
        while (!c->ncolors)
          assert(!pthread_cond_wait(&c->colorcond, &c->colormutex));
        for (; c->ncolors; c->ncolors--, k++)
          pixelcolor = v3add(pixelcolor, c->colors[c->ncolors]);
        assert(!pthread_mutex_unlock(&c->colormutex));
      }

      writecolor(stdout, pixelcolor, c->samplesperpixel);
    }
    fputc('.', stderr);
  }
  fputc('\n', stderr);
}

int main(void) {
  camera cam = CAMERADEFAULT;
  spherelist world = {0};
  int a, b;

  srandom(31415926);

  spherelistadd(&world,
                sphere(v3(0, -1000, 0), 1000, lambertian(v3(0.5, 0.5, 0.5))));

  for (a = -11; a < 11; a++) {
    for (b = -11; b < 11; b++) {
      float choosemat = randomfloat();
      vec3 center = v3add(v3(a, 0.2, b), v3mul(v3(0.9, 0, 0.9), v3random()));
      material mat;

      if (choosemat < 0.8)
        mat = lambertian(v3mul(v3random(), v3random()));
      else if (choosemat < 0.95)
        mat = metal(v3randominterval(0.5, 1), 0.5 * randomfloat());
      else
        mat = dielectric(1.5);

      spherelistadd(&world, sphere(center, 0.2, mat));
    }
  }

  spherelistadd(&world, sphere(v3(0, 1, 0), 1, dielectric(1.5)));
  spherelistadd(&world, sphere(v3(-4, 1, 0), 1, lambertian(v3(0.4, 0.2, 0.1))));
  spherelistadd(&world, sphere(v3(4, 1, 0), 1, metal(v3(0.7, 0.6, 0.5), 0)));

  cam.aspectratio = 16.0 / 9.0;
  cam.imagewidth = 1200;
  cam.samplesperpixel = 500;
  cam.maxdepth = 50;

  cam.vfov = 20;
  cam.lookfrom = v3(13, 2, 3);
  cam.lookat = v3(0, 0, 0);
  cam.vup = v3(0, 1, 0);

  cam.defocusangle = 0.6;
  cam.focusdist = 10;

  camerarender(&cam, &world);
  return 0;
}
