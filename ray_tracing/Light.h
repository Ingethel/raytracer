#pragma once

#include <algorithm>
#include "glm/glm.hpp"

/*
 * Class to hold light variables and calculation for different illuminations
 */
class Light{
public:
	Light();
	Light(float ambient, float diffuse, float constant_att,	float linear_att, float quadratic_att);
	~Light();
	float Attenuation(float distance);
	float Ambient_Light(float Ka);
	float Diffuse_Light(float Kd, glm::vec3 N, glm::vec3 L);
	float Specular_Light(float Ks, glm::vec3 N, glm::vec3 L, glm::vec3 V, float glossiness);
private:
	float ambient;
	float diffuse;
	float constant_att;
	float linear_att;
	float quadratic_att;
};
