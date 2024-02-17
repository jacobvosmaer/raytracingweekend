#include "camera.h"
#include "shared.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

struct interval {
  double min, max;
};

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

double randomdouble(void) { return rand() / (RAND_MAX + 1.0); }

void writecolor(FILE *out, vec3 color, int nsamples) {
  struct interval intensity = interval(0, 0.999);
  color = v3scale(color, 1.0 / nsamples);
  fprintf(out, "%d %d %d\n", (int)(256 * intervalclamp(intensity, color.x)),
          (int)(256 * intervalclamp(intensity, color.y)),
          (int)(256 * intervalclamp(intensity, color.z)));
}

typedef struct {
  vec3 p, normal;
  double t;
  int frontface;
} hitrecord;

/* outwardnormal must be unit vector */
void hitrecordsetnormal(hitrecord *rec, ray r, vec3 outwardnormal) {
  vec3 zero = {0};
  rec->frontface = v3dot(r.dir, outwardnormal) < 0;
  rec->normal = rec->frontface ? outwardnormal : v3sub(zero, outwardnormal);
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

vec3 raycolor(ray r, spherelist *world) {
  vec3 dir = v3unit(r.dir);
  double a = 0.5 * (dir.y + 1.0);
  hitrecord rec;
  if (spherelisthit(world, r, interval(0, INFINITY), &rec))
    return v3scale(v3add(rec.normal, v3(1, 1, 1)), 0.5);
  else
    return v3add(v3scale(v3(1, 1, 1), 1.0 - a), v3scale(v3(0.5, 0.7, 1), a));
}

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

void initialize(camera *c) {
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

  initialize(c);
  printf("P3\n%d %d\n255\n", c->imagewidth, c->imageheight);

  for (j = 0; j < c->imageheight; j++) {
    for (i = 0; i < c->imagewidth; i++) {
      vec3 pixelcolor = {0};
      int k;
      for (k = 0; k < c->samplesperpixel; k++)
        pixelcolor = v3add(pixelcolor, raycolor(getray(c, i, j), world));
      writecolor(stdout, pixelcolor, c->samplesperpixel);
    }
  }
}
