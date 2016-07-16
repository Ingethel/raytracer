#include "Object.h"

/*
 * 3 way find max
 */
float fmax(float f1,float f2, float f3) {
	float f = f1;
	if (f < f2) f = f2;
	if (f < f3) f = f3;
	return f;
}

/*
 * Calculate area of triangle with Heron's Formula
 */
float TriangleArea(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
{
	// compute sides of triangle
	float a = glm::length(p2-p1);
	float b = glm::length(p3-p2);
	float c = glm::length(p3-p1);
	// apply Heron's formula
	float s = (a+b+c)/2;
	float area = sqrt(s*(s-a)*(s-b)*(s-c));

	return area;
}

/*
 * Check if a point is inside a given triangle by comparing subsequent areas
 */
bool checkPointInArea(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p){
	float area = TriangleArea(p1, p2, p3);
	float area_diff = abs(area - (TriangleArea(p1, p2, p) + TriangleArea(p1, p, p3) + TriangleArea(p, p2, p3)));
	if (area_diff > area * 0.01)
		return false;
	return true;
}

/*
 * Default Material constructor
 */
Material::Material():
    ambient(.0f),
    diffuse(.0f),
    specular(.0f),
	glossiness(.0f),
	refraction(.0f)
{}

/*
 * Material constructor with defined properties
 */
Material::Material(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float glossiness, float reflection, float refraction):
    ambient(ambient), diffuse(diffuse), specular(specular),	glossiness(glossiness),
	reflection(reflection),	refraction(refraction){}

/*
 * Object constructor
 * Superclass
 */
Object::Object(const Material &material):
   material(material)
{}

/*
 * Sphere constructor
 * Subclass
 */
Sphere::Sphere(glm::vec3 center, float radius, const Material &material):
Object(material)
{
	this->centroid = center;
	this->radius = radius;
}
  
/* Polygon constructor
 * Subclass
 */
Plane::Plane(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 v4, const Material &material):
Object(material)
{
	if(!vertices.empty()) vertices.clear();
	// store vertices
	vertices.push_back(v1); 
	vertices.push_back(v2); 
	vertices.push_back(v3); 
	vertices.push_back(v4); 
	
	// temp vectors
	glm::vec3 u = vertices[1] - vertices[0];
	glm::vec3 v = vertices[2] - vertices[0];

	// normal
	normal = glm::cross(u, v);
	normal = glm::normalize(normal);

	// longer valid distance between two points of plane
	float a,b;
	a = glm::length(vertices[2]-vertices[0]);
	b = glm::length(vertices[3]-vertices[1]);
	radius = a > b ? a/2 : b/2;
	radius += 0.1;
	centroid = (vertices[0] + vertices[1] + vertices[2] + vertices[3]) / 4.f;
}

/*
 * Triangle constructor
 * Subclass
 */
Triangle::Triangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, const Material &material):
Object(material)
{
	if(!vertices.empty()) vertices.clear();
	// store vertices
	vertices.push_back(v1); 
	vertices.push_back(v2); 
	vertices.push_back(v3); 
	
	// temp vectors
	glm::vec3 u = vertices[1] - vertices[0];
	glm::vec3 v = vertices[2] - vertices[0];

	// normal
	normal = glm::cross(u, v);
	normal = glm::normalize(normal);

	// longer side of triangle
	float a,b,c;
	centroid = (vertices[0] + vertices[1] + vertices[2]) / 3.f;
	a = glm::length(vertices[0] - centroid);
	b = glm::length(vertices[1] - centroid);
	c = glm::length(vertices[2] - centroid);
	radius = fmax(a, b, c);
}

/*
 * Ray-Sphere Intersection
 */
bool Sphere::Intersect(const Ray &ray, IntersectInfo &info, float MAX) const {
	// check intersection by discriminant (B^2 - 4AC)
	glm::vec3 e_c = ray.origin - this->centroid;
	float _b = glm::dot(ray.direction, e_c);
	float B = _b*2.f;
	float A = glm::dot(ray.direction,ray.direction);
	float C = glm::dot(e_c,e_c) - pow(this->radius,2);
	float discriminant = B*B - A*C*4;

	/*
	 * if discriminant = 0 -> one hit point
	 * if discriminant > 0 -> 2 hit points	(though we only care for one since the second will be obscured)
	 * if discriminant < 0 -> no hit point
	 */
	if (discriminant >= 0){
		float t1 = (-_b + sqrt(_b*_b-A*C))/A;
		if(t1 < 0.f)  t1 = std::numeric_limits<float>::infinity();
		float t2 = (-_b - sqrt(_b*_b-A*C))/A; 
		if(t2 < 0.f)  t2 = std::numeric_limits<float>::infinity();
		float t = (t1 < t2) ? t1 : t2;

		// check if point is between origin and light
		if(t > 0.0f && glm::length(glm::vec3(ray(t))-ray.origin) < MAX)
			// check if it is closer than any previous intersection
			if(t < info.time){
				info.time = t;
				info.hitPoint = ray(info.time);
				info.material = &this->material;
				info.normal = (info.hitPoint - centroid)/radius;
				return true;
			}
	}
	return false;
}

/* 
 * Ray-Plane intersection 
 */
bool Plane::Intersect(const Ray &ray, IntersectInfo &info, float MAX) const {
	// check if ray is not parallel to polygon
	if(glm::dot(ray.direction,this->normal) != 0.f){
		// calculate t coefficient of line being axis aligned with polygon
		float t = glm::dot((vertices[0] - ray.origin),this->normal)/glm::dot(ray.direction,this->normal);
		glm::vec3 point = ray(t);
		
		// check if point is between origin and light
		if(t < 0.0f || glm::length(point - ray.origin) > MAX){ return false; } 			

		// check by rough estimate, ignores points that are aligned with plane, but far from it
		if(glm::distance(centroid, point) > radius){ return false; }

		// check if its actually inside polygon
		if(checkPointInArea(vertices[0], vertices[1], vertices[2], point) || (checkPointInArea(vertices[0], vertices[2], vertices[3], point)))
			
			// check if it is closer than any previous intersection
			if(t < info.time){
				info.time = t;
				info.hitPoint = ray(info.time);
				info.material = &this->material;
				info.normal = this->normal;
				return true;
			}
		
	}
	return false; 
}

/*
 * Ray-Triangle intersection
 */
bool Triangle::Intersect(const Ray &ray, IntersectInfo &info, float MAX) const { 
	// check if ray is not parallel to triangle
	if(glm::dot(ray.direction,this->normal) != 0.f){
		// calculate t coefficient of line being axis aligned with triangle
		float t = glm::dot((vertices[0] - ray.origin),this->normal)/glm::dot(ray.direction,this->normal);
		glm::vec3 point = ray(t);

		// check if point is between origin and light
		if(t < 0.0f || glm::length(point-ray.origin) > MAX){ return false; }
			
		// check by rough estimate, ignores points that are aligned with plane, but far from it
		if (glm::distance(centroid, point) > radius){ return false; }

		// check if its actually inside triangle
		if(checkPointInArea(vertices[0], vertices[1], vertices[2], point))
			
			// check if it is closer than any previous intersection
			if(t < info.time){
				info.time = t;
				info.hitPoint = ray(info.time);
				info.material = &this->material;
				info.normal = this->normal;
				return true;
			}
		
	}
	return false; 
}
