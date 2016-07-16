#pragma once

#include "Ray.h"
#include <vector>
#include <iostream>
/*
 * Material class
 * keeps material properties
 */
class Material {
public:
	Material();
	Material(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float glossiness, float reflection, float refraction);

	/*
	 * getter functions for reading member variables from outside
	 */
	float getAmbient(int channel) const { return ambient[channel]; };
	float getDiffuse(int channel) const { return diffuse[channel]; };
	float getSpecular(int channel) const { return specular[channel]; };
	float getGlossiness() const { return glossiness; };
	float getReflectivity() const { return reflection; };
	float getRefraction() const { return refraction; };

protected:
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float glossiness;
	float reflection;
	float refraction;
};

/*
 * Object Class
 * keeps material properties
 */
class Object {
public:
	Object(const Material &material);
	virtual bool Intersect(const Ray &ray, IntersectInfo &info, float MAX) const { return false; }
protected:
	glm::vec3 centroid;
	Material material;
	float radius;
};

/*
 * Sphere Object
 * keeps centre position, radius and material properties
 */
class Sphere : public Object {
public:
	Sphere(glm::vec3 center, float radius, const Material &material);
	bool Intersect(const Ray &ray, IntersectInfo &info, float MAX) const;
};

/*
 * Polygon Object
 * keeps list of vertices, normal and material properties
 */
class Plane : public Object {
public:
	Plane(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 v4, const Material &material);
	bool Intersect(const Ray &ray, IntersectInfo &info, float MAX) const;
private:
	std::vector<glm::vec3> vertices;
	glm::vec3 normal;
};

/*
 * Triangle Object
 * keeps list of vertices, normal and material properties
 */
class Triangle : public Object {
public:
	Triangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, const Material &material);
	bool Intersect(const Ray &ray, IntersectInfo &info, float MAX) const;
private:
	std::vector<glm::vec3> vertices;
	glm::vec3 normal;
};