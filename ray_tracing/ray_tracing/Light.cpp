#include "Light.h"

Light::Light()
{}

/*
 * Create Light with specified properties
 */
Light::	Light(float ambient, float diffuse, float constant_att,	float linear_att, float quadratic_att)
{
	this->ambient = ambient;
	this->diffuse = diffuse;
	this->constant_att = constant_att;
	this->linear_att = linear_att;
	this->quadratic_att = quadratic_att;
	// constant illumination as default
	if(this->linear_att == 0.f && this->quadratic_att == 0.f)
		this->constant_att = 1.f;
}

Light::~Light()
{}

float Light::Attenuation(float distance){
	return 1.f / (this->constant_att + this->linear_att*distance + this->quadratic_att*pow(distance,2));
}

/*
 * calculates the ambient light intensity
 */
float Light::Ambient_Light(float Ka){
	return this->ambient * Ka;
}

/*
 * calculates the diffuse light intensity
 */
float Light::Diffuse_Light(float Kd, glm::vec3 N, glm::vec3 L){
	float theta = std::max(0.0f, glm::dot(N,L));	
	return this->diffuse * Kd * theta;
}

/*
 * calculates the specular light intensity
 */
float Light::Specular_Light(float Ks, glm::vec3 N, glm::vec3 L, glm::vec3 V, float glossiness){
	float coef = 2 * glm::dot(N,L);
	glm::vec3 R;
	R = L - N*coef;
	float alpha = std::max(0.0f, glm::dot(R, V));
	return this->diffuse * Ks * pow(alpha, glossiness);
}
