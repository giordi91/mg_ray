#include "world.h"

void World::rayCast(Image img, int samplePerPixel) {

  vec3 camZ = glm::normalize(vec3(0.0f, 0.1f, 1.0f));
  vec3 up(0.0f, 1.0f, 0.0f);
  vec3 camX = glm::normalize(glm::cross(up, camZ));
  vec3 camY = glm::normalize(glm::cross(camZ, camX));
  vec3 camP(0.0f, 2.0f, 10.0f);
  float filmDist = 1.0f;
  vec3 filmCenter = camP + (filmDist * (-camZ));

  float ratio = float(img.height) / float(img.width);
  float filmWidth = 1.0f;
  float filmHeight = filmWidth * ratio;
  float halfFilmWidth = 0.5f * filmWidth;
  float halfFilmHeight = 0.5f * filmHeight;

  const float contrib = 1.0f / samplePerPixel;
  vec3 rayOrigin = camP;
  for (int h = 0; h < img.height; ++h) {

    float filmY = -1.0f + 2.0f * (float(h) / float(img.height));

    for (int w = 0; w < img.width; ++w) {
      float filmX = -1.0f + 2.0f * (float(w) / float(img.width));

      vec3 col = vec3(0.0f, 0.0f, 0.0f);
      for (int s = 0; s < samplePerPixel; ++s) {
		float sFilmY = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))/float(img.height) + filmY;
		float sFilmX = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))/float(img.width) + filmX;
		//  float sFilmY = filmY;
		//  float sFilmX = filmX;

        vec3 currentCol = vec3(0.0f, 0.0f, 0.0f);
        vec3 filmP = filmCenter + (sFilmX * halfFilmWidth * camX) +
                     (sFilmY * halfFilmHeight * camY);

		
        vec3 rayDirection = glm::normalize(filmP - camP);

        float hitDistance = 999999.0f;
        for (auto &hitobj : m_hitable) {
          float thisDistance = hitobj->rayCast(rayOrigin, rayDirection);

          if ((thisDistance > 0) && (thisDistance < hitDistance)) {
            hitDistance = thisDistance;
            // lets compute the point on the sphere with the t
            // vec3 p = rayOrigin + thisDistance * rayDirection;
            // vec3 norm = glm::normalize(p - s.position);
            // result = 0.5f * vec3{ norm.x + 1.0, norm.y + 1.0f, norm.z + 1.0
            // };
            currentCol = hitobj->mat->color ;
          }
        }
		col += currentCol*contrib;
      }

      // writing to image
      int finalIndex = ((img.height - h - 1) * img.width + w) * 4;
      // TODO(giordi) change this to write a single value as int32
      img.data[finalIndex + 0] = 255.99f * col.x;
      img.data[finalIndex + 1] = 255.99f * col.y;
      img.data[finalIndex + 2] = 255.99f * col.z;
      img.data[finalIndex + 3] = 255;
    }
  }
}
