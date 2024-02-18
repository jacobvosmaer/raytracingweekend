#ifndef VEC3_H
#define VEC3_H

typedef struct {
  double x, y, z;
} vec3;

vec3 v3(double x, double y, double z);
vec3 v3add(vec3 v, vec3 w);
vec3 v3sub(vec3 v, vec3 w);
vec3 v3neg(vec3 v);
vec3 v3mul(vec3 v, vec3 w);
vec3 v3scale(vec3 v, double c);
double v3dot(vec3 v, vec3 w);
double v3length(vec3 v);
vec3 v3unit(vec3 v);
vec3 v3cross(vec3 v, vec3 w);
vec3 v3random(void);
vec3 v3randominterval(double min, double max);
vec3 v3randomunit(void);
vec3 v3randominunitdisk(void);

#endif
