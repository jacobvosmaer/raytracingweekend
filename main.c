#include "shared.h"

#include <math.h>
#include <stdio.h>

void writecolor(FILE *out, vec3 color) {
  color = v3scale(color, 255.999);
  fprintf(out, "%d %d %d\n", (int)(color.x), (int)(color.y), (int)(color.z));
}

vec3 rayat(ray r, double t) { return v3add(r.orig, v3scale(r.dir, t)); }

typedef struct {
  vec3 p, normal;
  double t;
  int frontface;
} hitrecord;

/* outwardnormal must be unit vector */
void hitrecordsetnormal(hitrecord *rec, ray r, vec3 outwardnormal) {
  rec->frontface = v3dot(r.dir, outwardnormal) < 0;
  rec->normal = rec->frontface ? outwardnormal : v3scale(outwardnormal, -1);
}

int spherehit(struct sphere sp, ray r, double tmin, double tmax,
              hitrecord *rec) {
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
  if (root <= tmin || root >= tmax) {
    root = (-halfb + sqrtd) / a;
    if (root <= tmin || root >= tmax)
      return 0;
  }

  rec->t = root;
  rec->p = rayat(r, rec->t);
  hitrecordsetnormal(rec, r,
                     v3scale(v3sub(rec->p, sp.center), 1.0 / sp.radius));
  return 1;
}

int spherelisthit(spherelist *sl, ray r, double tmin, double tmax,
                  hitrecord *rec) {
  int i, nhit = 0;
  double closest = tmax;

  for (i = 0; i < sl->n; i++) {
    if (spherehit(sl->spheres[i], r, tmin, closest, rec)) {
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
  if (spherelisthit(world, r, 0, INFINITY, &rec))
    return v3scale(v3add(rec.normal, v3(1, 1, 1)), 0.5);
  else
    return v3add(v3scale(v3(1, 1, 1), 1.0 - a), v3scale(v3(0.5, 0.7, 1), a));
}

ray rayfromto(vec3 from, vec3 to) {
  ray r;
  to = v3sub(to, from);
  r.orig = from;
  r.dir = to;
  return r;
}

int main(void) {
  double aspectratio = 16.0 / 9.0, focallength = 1, viewportheight = 2,
         viewportwidth;
  int imagewidth = 2048, imageheight = imagewidth / aspectratio, i, j;
  vec3 cameracenter = {0}, viewportu, viewportv, pixeldu, pixeldv,
       viewportupperleft, pixel00loc;
  spherelist world = {0};

  spherelistadd(&world, sphere(v3(0, 0, -1), 0.5));
  spherelistadd(&world, sphere(v3(0, -100.5, -1), 100));

  if (imageheight < 1)
    imageheight = 1;
  viewportwidth = viewportheight * ((double)imagewidth / imageheight);
  viewportu = v3(viewportwidth, 0, 0);
  viewportv = v3(0, -viewportheight, 0);
  pixeldu = v3scale(viewportu, 1.0 / imagewidth);
  pixeldv = v3scale(viewportv, 1.0 / imageheight);

  viewportupperleft = v3add(v3add(v3add(cameracenter, v3(0, 0, -focallength)),
                                  v3scale(viewportu, -0.5)),
                            v3scale(viewportv, -0.5));
  pixel00loc = v3add(viewportupperleft, v3scale(v3add(pixeldu, pixeldv), 0.5));

  printf("P3\n%d %d\n255\n", imagewidth, imageheight);
  for (j = 0; j < imageheight; j++) {
    for (i = 0; i < imagewidth; i++) {
      vec3 pixelcenter =
          v3add(v3add(pixel00loc, v3scale(pixeldu, i)), v3scale(pixeldv, j));
      ray r = rayfromto(cameracenter, pixelcenter);
      vec3 pixelcolor = raycolor(r, &world);
      writecolor(stdout, pixelcolor);
    }
  }
  return 0;
}
