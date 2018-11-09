#include "hitable.h"

float tolerance = 0.00001f;
float Plane::rayCast(vec3 rayOrigin, vec3 rayDirection) const
{

  float denominator = glm::dot(normal, rayDirection);
  if (abs(denominator) < tolerance) {
    return 999999.0f;
  }
  return (-d - glm::dot(normal, rayOrigin)) / (denominator);
}

float Sphere::rayCast(vec3 rayOrigin, vec3 rayDirection) const
{
  vec3 sphereLocalOrigin = rayOrigin - position;
  //vec3 sphereLocalOrigin = rayOrigin ;
  float a = glm::dot(rayDirection, rayDirection);
  float b = 2.0f * glm::dot(rayDirection, sphereLocalOrigin);
  float c = glm::dot(sphereLocalOrigin, sphereLocalOrigin) - (radius * radius);
  float denom = 2.0f * a;
  float rootTerm = sqrt(b * b - (4.0f * a * c));
  if (abs(rootTerm) > tolerance) {
    float tp = (-b + rootTerm) / denom;
    float tn = (-b - rootTerm) / denom;
    float t = tp;
    if ((tn > 0) && (tn < tp)) {
      t = tn;
    }
    return t;

  } else {
    return 999999.0f;
  }
}
