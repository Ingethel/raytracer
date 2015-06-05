#ifndef RayTracer_h
#define RayTracer_h

#define _USE_MATH_DEFINES
#include <GL/glut.h>
#include "Ray.h"
#include "Object.h"
#include "Light.h"

// window dimensions
const int windowX = 640;
const int windowY = 480;

// camera position
glm::vec3 camera_pos(-10.f,10.f,10.f);
// light position
glm::vec3 light_pos(-6.f,4.f,3.f);
// create light source
Light light_0(.7f, 1.f, .0f, .3f, .0f);

// list of objects
std::vector<Object*> objects;

/*
 * list of objects that can cast shadow
 *
 * some objects may not cast shadow or may be simplified
 * (ex. chess floor pattern can be reduced to one plane instead of 100)
 */
std::vector<Object*> can_cast_shadow;

// number of bounces allowed
const int MAX_BOUNCES = 2;

// air refraction coefficient
const float air_ref = 1.f;

// white coloured reflective material
Material white(glm::vec3(.2f,.2f,.2f),  // ambient 
	       glm::vec3(.5f,.5f,.5f),		// diffuse
	       glm::vec3(1.f,1.f,1.f),		// specular
	       20.f,						// glossiness
	       .5f,							// reflectivity
	      .0f);							// refractivity

// black coloured reflective material
Material black(glm::vec3(.0f,.0f,.0f),  // ambient 
	       glm::vec3(.1f,.1f,.1f),		// diffuse
	       glm::vec3(.3f,.3f,.3f),		// specular
	       3.f,							// glossiness
	       .1f,							// reflectivity
	       .0f);						// refractivity

// approximation of mirror 
Material mirror(glm::vec3(.0f,.0f,.0f), // ambient 
		glm::vec3(.0f,.0f,.0f),			// diffuse
		glm::vec3(.9f,.9f,.9f),			// specular
		7.f,							// glossiness
		1.f,							// reflectivity
		.0f);							// refractivity

// declaration for recursion
glm::vec3 CastRay(Ray &ray, Payload &payload, IntersectInfo &info);

#endif
