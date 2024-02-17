#include "camera.h"
#include "shared.h"

material lambertian(vec3 albedo) {
  material mat;
  mat.type = LAMBERTIAN;
  mat.albedo = albedo;
  return mat;
}

material metal(vec3 albedo) {
  material mat;
  mat.type = METAL;
  mat.albedo = albedo;
  return mat;
}

int main(void) {
  camera cam = CAMERADEFAULT;
  spherelist world = {0};
  material matground = lambertian(v3(0.8, 0.8, 0)),
           matcenter = lambertian(v3(0.7, 0.3, 0.3)),
           matleft = metal(v3(0.8, 0.8, 0.8)),
           matright = metal(v3(0.8, 0.6, 0.2));

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
