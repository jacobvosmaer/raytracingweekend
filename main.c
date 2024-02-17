#include "camera.h"
#include "shared.h"

int main(void) {
  camera cam = CAMERADEFAULT;
  spherelist world = {0};
  material dummy = {0};

  spherelistadd(&world, sphere(v3(0, 0, -1), 0.5, dummy));
  spherelistadd(&world, sphere(v3(0, -100.5, -1), 100, dummy));

  cam.aspectratio = 16.0 / 9.0;
  cam.imagewidth = 400;
  cam.samplesperpixel = 100;
  cam.maxdepth = 50;
  camerarender(&cam, &world);

  return 0;
}
