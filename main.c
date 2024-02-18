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

double pi = 3.1415926537;

double degtorad(double deg) { return pi * deg / 180.0; }

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

vec3 v3randomunitdisk(void) {
  vec3 v;
  do
    v = v3add(v3(-1, -1, 0), v3scale(v3(randomdouble(), randomdouble(), 0), 2));
  while (v3dot(v, v) > 1);
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
    return 0; /* The ray does not hit the sphere */

  /* The ray hits the sphere in 1 or 2 points (roots). Do any of the roots lie
   * in the interval? */
  sqrtd = sqrt(discriminant);
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
  double closest = t.max;

  for (i = 0; i < sl->n; i++) {
    if (spherehit(sl->spheres[i], r, interval(t.min, closest), rec)) {
      hit = 1;
      closest = rec->t;
    }
  }
  return hit;
}

vec3 reflect(vec3 v, vec3 n) { return v3sub(v, v3scale(n, 2 * v3dot(v, n))); }

vec3 refract(vec3 uv, vec3 n, double etaioveretat) {
  double costheta = fmin(v3dot(v3neg(uv), n), 1);
  vec3 routperp = v3scale(v3add(uv, v3scale(n, costheta)), etaioveretat),
       routparallel = v3scale(n, -sqrt(fabs(1.0 - v3dot(routperp, routperp))));
  return v3add(routperp, routparallel);
}

double reflectance(double cosine, double refidx) {
  double r0 = (1.0 - refidx) / (1.0 + refidx);
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
    double refractionratio = rec->frontface ? 1.0 / data.ir : data.ir;
    vec3 unitdirection = v3unit(in.dir);
    double costheta = fmin(v3dot(v3neg(unitdirection), rec->normal), 1.0),
           sintheta = sqrt(1.0 - costheta * costheta);
    scattered->orig = rec->p;
    scattered->dir =
        refractionratio * sintheta > 1.0 ||
                reflectance(costheta, refractionratio) > randomdouble()
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
    double a = 0.5 * (dir.y + 1.0);
    return v3add(v3scale(v3(1, 1, 1), 1.0 - a), v3scale(v3(0.5, 0.7, 1), a));
  }
}

typedef struct {
  double aspectratio;
  int imagewidth, samplesperpixel, maxdepth;
  double vfov;
  vec3 lookfrom, lookat, vup;
  double defocusangle, focusdist;

  /* derived values */
  int imageheight;
  vec3 center, pixeldu, pixeldv, pixel00loc, u, v, w, defocusdisku,
      defocusdiskv;
} camera;

#define CAMERADEFAULT                                                          \
  { 1, 100, 10, 10, 90, {0, 0, -1}, {0, 0, 0}, {0, 1, 0}, 0, 10 }

vec3 pixelsamplesquare(camera *c) {
  double px = -0.5 + randomdouble(), py = -0.5 + randomdouble();
  return v3add(v3scale(c->pixeldu, px), v3scale(c->pixeldv, py));
}

vec3 defocusdisksample(camera *c) {
  vec3 p = v3randomunitdisk();
  return v3add(c->center, v3add(v3scale(c->defocusdisku, p.x),
                                v3scale(c->defocusdiskv, p.y)));
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
  double viewportheight, viewportwidth, h, defocusradius;
  vec3 viewportu, viewportv, viewportupperleft;

  c->imageheight = c->imagewidth / c->aspectratio;
  if (c->imageheight < 1)
    c->imageheight = 1;

  c->center = c->lookfrom;

  h = tan(degtorad(c->vfov) / 2);
  viewportheight = 2 * h * c->focusdist;
  viewportwidth = viewportheight * ((double)c->imagewidth / c->imageheight);

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
  int a, b;

  spherelistadd(&world,
                sphere(v3(0, -1000, 0), 1000, lambertian(v3(0.5, 0.5, 0.5))));

  for (a = -11; a < 11; a++) {
    for (b = -11; b < 11; b++) {
      double choosemat = randomdouble();
      vec3 center = v3(a + 0.9 * randomdouble(), 0.2, b + 0.9 * randomdouble());
      material mat;

      if (choosemat < 0.8)
        mat = lambertian(v3mul(v3random(), v3random()));
      else if (choosemat < 0.95)
        mat = metal(v3add(v3(0.5, 0.5, 0.5), v3scale(v3random(), 0.5)),
                    0.5 * randomdouble());
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
