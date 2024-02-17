#include <stdio.h>

int main(void) {
  int imagewidth = 256, imageheight = 256, i, j;

  printf("P3\n%d %d\n255\n", imagewidth, imageheight);
  for (j = 0; j < imageheight; j++) {
    for (i = 0; i < imagewidth; i++) {
      double r = (double)i / (imagewidth - 1),
             g = (double)j / (imageheight - 1), b = 0;
      int ir = 255.999 * r, ig = 255.999 * g, ib = 255.999 * b;
      printf("%d %d %d\n", ir, ig, ib);
    }
  }
}
