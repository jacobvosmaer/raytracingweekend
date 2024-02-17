#ifndef CAMERA_H
#define CAMERA_H

#include "shared.h"

typedef struct {
  double aspectratio;
  int imagewidth, imageheight;
  vec3 center, pixeldu, pixeldv, pixel00loc;
} camera;

#define CAMERADEFAULT                                                          \
  { 1.0, 100 }

void camerarender(camera *c, spherelist *world);

#endif
