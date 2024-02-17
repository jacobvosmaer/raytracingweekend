#include <math.h>
#include <stdio.h>

typedef struct {
  double x, y, z;
} vec3;

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

void writecolor(FILE *out, vec3 color) {
  double s = 255.999;
  fprintf(out, "%d %d %d\n", (int)(s * color.x), (int)(s * color.y),
          (int)(s * color.z));
}

typedef struct {
  vec3 orig, dir;
} ray;

vec3 rayat(ray r, double t) { return v3add(r.orig, v3scale(r.dir, t)); }

vec3 raycolor(ray r) {
  vec3 dir = v3scale(r.dir, 1.0 / v3length(r.dir));
  double a = 0.5 * (dir.y + 1.0);
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
  int imagewidth = 256, imageheight = imagewidth / aspectratio, i, j;
  vec3 cameracenter = {0}, viewportu, viewportv, pixeldu, pixeldv,
       viewportupperleft, pixel00loc;

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
      vec3 pixelcolor = raycolor(r);
      writecolor(stdout, pixelcolor);
    }
  }
}
