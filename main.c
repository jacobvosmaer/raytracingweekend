#include <stdio.h>

typedef struct {
  double x, y, z;
} vec3;

vec3 v3(double x,double y,double z){
vec3 v;
v.x=x;v.y=y;v.z=z;return v;}

void writecolor(FILE *out, vec3 color) {
  double s = 255.999;
  fprintf(out, "%d %d %d\n", (int)(s * color.x), (int)(s * color.y),
          (int)(s * color.z));
}

int main(void) {
  int imagewidth = 256, imageheight = 256, i, j;

  printf("P3\n%d %d\n255\n", imagewidth, imageheight);
  for (j = 0; j < imageheight; j++) {
    for (i = 0; i < imagewidth; i++) {
      vec3 pixelcolor = v3((double)i / (imagewidth - 1),
                         (double)j / (imageheight - 1), 0);
      writecolor(stdout, pixelcolor);
    }
  }
}
