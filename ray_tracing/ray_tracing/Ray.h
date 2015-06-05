#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class Material;

class Ray {
  public:
	// start point
    glm::vec3 origin;
	// end point
    glm::vec3 direction;

    Ray(const glm::vec3 &origin, const glm::vec3 &direction):
      origin(origin),
      direction(direction)
    {}

    /* 
	 * Parametric expression of line
	 * Returns the position of the ray at time t 
	 */
    glm::vec3 operator() (const float &t) const {
      return origin + direction*t;
    }
};

class IntersectInfo {
  public:
    IntersectInfo():
      time(std::numeric_limits<float>::infinity()),
      hitPoint(0.0f),
      normal(0.0f),
      material(NULL)
    {}

    // The position of intersection
    glm::vec3 hitPoint;
    // The normal vector of the surface at the point of the intersection
    glm::vec3 normal;
    // The time along the ray that the intersection occurs
    float time;
    // The material of the object that was intersected
    const Material *material;
	
	// overload
    IntersectInfo &operator =(const IntersectInfo &rhs) {
      hitPoint = rhs.hitPoint;
      material = rhs.material;
      normal = rhs.normal;
      time = rhs.time;
	  return *this;
    }
};

class Payload {
  public:
    Payload():
      color(0.0f),
      numBounces_reflect(0.f),
      numBounces_refract(0.f)
    {}
    
    glm::vec3 color;
    float numBounces_reflect;
	float numBounces_refract;
};