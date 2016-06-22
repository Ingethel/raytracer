#include "RayTracer.h"

/*
 * Free Memory
 */
void cleanup() {
	for(unsigned int i = 0; i < objects.size(); ++i){
		if(objects[i]){
			delete objects[i];
		}
	}
	for(unsigned int i = 0; i < can_cast_shadow.size(); ++i){
		if(can_cast_shadow[i]){
			delete can_cast_shadow[i];
		}
	}
}

/*
 * Runs through all objects and check if intersects with current ray
 * original ray
 */
bool CheckIntersection(const Ray &ray, IntersectInfo &info) {
	bool flag = false;
	for(int i = 0; i < objects.size(); i++)
		if(objects[i]->Intersect(ray,info, std::numeric_limits<float>::infinity()))
			flag = true;
		
	return flag;
}

/*
 * Runs through all objects and check if intersects with current ray
 * shadow ray
 */
bool CheckIntersection_Shadow(const Ray &ray, IntersectInfo &info) {
	bool flag = false;
	float length_toLight = glm::length(light_pos - ray.origin);
	for(int i = 0; i < can_cast_shadow.size(); i++)
		if(can_cast_shadow[i]->Intersect(ray, info, length_toLight))
			flag = true;
		
	return flag;
}

/*
 * Calculate colour at given pixel
 */
glm::vec3 calculateColor(IntersectInfo &info, Light &light, bool shadow_flag){
	float r,g,b;

	if(shadow_flag){
		// calculate shadowed light for each colour channel
		r = light.Ambient_Light(info.material->getAmbient(0));
		g = light.Ambient_Light(info.material->getAmbient(1));
		b = light.Ambient_Light(info.material->getAmbient(2));
	}
	else{
		// normalised vector from light position to current point
		glm::vec3 lightToVertexUnitVector = glm::normalize(light_pos - info.hitPoint);
		// normalised vector from camera position to current point
		glm::vec3 cameraToVertexUnitVector = glm::normalize(camera_pos - info.hitPoint);
		// distance to light source
		float distance = glm::length(light_pos - info.hitPoint);
		// attenuation
		float attenuation = light.Attenuation(distance);
		// calculate light for each colour channel
		r = light.Ambient_Light(info.material->getAmbient(0)) + 
			attenuation * (light.Diffuse_Light(info.material->getDiffuse(0),info.normal, lightToVertexUnitVector) + 
			light.Specular_Light(info.material->getSpecular(0), info.normal, -lightToVertexUnitVector, cameraToVertexUnitVector, info.material->getGlossiness()));
		g = light.Ambient_Light(info.material->getAmbient(1)) + 
			attenuation * (light.Diffuse_Light(info.material->getDiffuse(1),info.normal, lightToVertexUnitVector) + 
			light.Specular_Light(info.material->getSpecular(1), info.normal, -lightToVertexUnitVector, cameraToVertexUnitVector, info.material->getGlossiness()));
		b = light.Ambient_Light(info.material->getAmbient(2)) + 
			attenuation * (light.Diffuse_Light(info.material->getDiffuse(2),info.normal, lightToVertexUnitVector) + 
			light.Specular_Light(info.material->getSpecular(2), info.normal, -lightToVertexUnitVector, cameraToVertexUnitVector, info.material->getGlossiness()));
	}

	return glm::vec3(r,g,b);
}

/*
 * Creates ray to check for shadows and calculate colour
 */
glm::vec3 checkLight(IntersectInfo &info){
	// temp variables for checking illumination
	Ray check_luminance(info.hitPoint+info.normal*.1f,light_pos);
	IntersectInfo temp;

	/*
	 * check if current point is visible by light source
	 * if new casted ray intersects with some object then point is occluded,
	 *		calculate only ambient illumination
	 * else calculate full illumination
	 */
	return calculateColor(info, light_0, CheckIntersection_Shadow(check_luminance, temp));

}

/*
 * Calculates the direction of the reflected ray
 */
Ray reflect(const Ray &ray, IntersectInfo &info){
	glm::vec3 originToPoint = glm::normalize(ray.origin - info.hitPoint);
	glm::vec3 R = info.normal* 2.f * glm::dot(info.normal,originToPoint) - originToPoint;

	return Ray(info.hitPoint+info.normal*0.1f, R);
}

/*
 * Calculates the direction of the refracted ray
 */
Ray refract(const Ray &ray, IntersectInfo &info){
	float mat_dif = glm::dot(info.normal,ray.direction) < 0 ? air_ref/info.material->getRefraction() : info.material->getRefraction()/air_ref;
	glm::vec3 norm = glm::dot(info.normal,ray.direction) < 0 ? info.normal : -info.normal;
	float cosTheta = -glm::dot(norm, ray.direction);
	float cosPhi2 = 1.f - mat_dif*mat_dif * (1.f - cosTheta*cosTheta);
	glm::vec3 direction = mat_dif*ray.direction + (mat_dif*cosTheta - sqrtf(cosPhi2))*norm;
	
	return Ray(info.hitPoint+direction*.1f, direction);
}

/*
 * Creates and casts reflected rays
 */
void CastReflection(Ray &ray, Payload &payload, IntersectInfo info){
	if(payload.numBounces_reflect<MAX_BOUNCES)
		if(info.material->getReflectivity()>0.f){
			payload.numBounces_reflect++;
			IntersectInfo temp;
			Ray reflected_ray = reflect(ray, info);
			if(CheckIntersection(reflected_ray, temp)){
				payload.color += checkLight(temp) * info.material->getReflectivity();
				CastRay(reflected_ray, payload, temp);
			}
		}
}

/*
 * Creates and casts refracted rays
 */
void CastRefraction(Ray &ray, Payload &payload, IntersectInfo info){
	if(info.material->getRefraction()>0.f){
		if(payload.numBounces_refract<MAX_BOUNCES){
			payload.numBounces_refract++;
			IntersectInfo temp;
			Ray refracted_ray = refract(ray, info);
			if(CheckIntersection(refracted_ray, temp))
					CastRay(refracted_ray, payload, temp);
		}
	}
	else if(payload.numBounces_refract>0){
		payload.color += checkLight(info);
	}
}

/*
 * Casts original ray and calls subsequent methods
 */
glm::vec3 CastRay(Ray &ray, Payload &payload, IntersectInfo &info) {
	
	CastRefraction(ray, payload, info);
	CastReflection(ray, payload, info);

	// check out of bounds
	for(int i = 0; i < 3; i++)
		payload.color[i] = std::min(1.f, payload.color[i]);
		
	return payload.color;
}

#pragma region Thread Methods
void DrawOutput(OUTPUT &o){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBegin(GL_POINTS);
	for (int x = 0; x < windowX; x++) {
		for (int y = 0; y < windowY; y++) {
			int index = x + y * windowX;
			glColor3f(o.pixel_r[index], o.pixel_g[index], o.pixel_b[index]);
			glVertex3f(x, y, 0);
		}
	}
	glEnd();
	glFlush();
	glutSwapBuffers();
}

void *thread_work(void *arg){
	int offset = (int)arg;
	int start_loop = 0 + offset;
	int end_loop = windowX*windowY;

	glm::vec3 colour;
	glm::mat4 viewMatrix = glm::lookAt(glm::vec3(-10.0f, 10.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 projMatrix = glm::perspective(45.0f, (float)windowX / (float)windowY, 1.0f, 10000.0f);


	for (int i = start_loop; i < end_loop; i += NUMTHREADS){
		int x = i % windowX;
		int y = i / windowX;

		float pixelX = 2 * ((x + 0.5f) / windowX) - 1;
		float pixelY = -2 * ((y + 0.5f) / windowY) + 1;

		glm::vec4 worldNear = glm::inverse(viewMatrix) * glm::inverse(projMatrix) * glm::vec4(pixelX, pixelY, -1, 1);
		glm::vec4 worldFar = glm::inverse(viewMatrix) * glm::inverse(projMatrix) * glm::vec4(pixelX, pixelY, 1, 1);

		glm::vec3 worldNearPos = glm::vec3(worldNear.x, worldNear.y, worldNear.z) / worldNear.w;
		glm::vec3 worldFarPos = glm::vec3(worldFar.x, worldFar.y, worldFar.z) / worldFar.w;

		Payload payload;
		IntersectInfo info;
		Ray ray(worldNearPos, glm::normalize(glm::vec3(worldFarPos - worldNearPos)));

		if (CheckIntersection(ray, info)) {
			payload.color += checkLight(info);
			colour = CastRay(ray, payload, info);
		}
		else
			colour = glm::vec3(0, 0, 0);
		
		scene.pixel_r[i] = colour.r;
		scene.pixel_g[i] = colour.g;
		scene.pixel_b[i] = colour.b;
	}

	pthread_exit((void*)0);
	return NULL;
}

void render_threads(){
	std::clock_t start = std::clock();
	void *status;
	// start threads
	for (int i = 0; i < NUMTHREADS; i++){
		pthread_create(&callThd[i], &attr, thread_work, (void *)i);
	}
	// wait to finish
	for (int i = 0; i < NUMTHREADS; i++){
		pthread_join(callThd[i], &status);
	}
	// draw output
	DrawOutput(scene);
	std::cout << "Done " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 100) / 100 << " s" << std::endl;
}

void initialise_thread_variables(){
	scene.pixel_r = (float*)malloc(NUMTHREADS*windowX*windowY*sizeof(float));
	scene.pixel_g = (float*)malloc(NUMTHREADS*windowX*windowY*sizeof(float));
	scene.pixel_b = (float*)malloc(NUMTHREADS*windowX*windowY*sizeof(float));

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
}
#pragma endregion
/*
 * Render Function
 */
void Render() {
	std::clock_t start = std::clock();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 viewMatrix = glm::lookAt(glm::vec3(-10.0f,10.0f,10.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f));
	glm::mat4 projMatrix = glm::perspective(45.0f, (float)windowX / (float)windowY, 1.0f, 10000.0f);

	glBegin(GL_POINTS);
  
	for(int x = 0; x < windowX; ++x)
  		for(int y = 0; y < windowY; ++y){

      			float pixelX =  2*((x+0.5f)/windowX)-1;
      			float pixelY = -2*((y+0.5f)/windowY)+1;
      
      			glm::vec4 worldNear = glm::inverse(viewMatrix) * glm::inverse(projMatrix) * glm::vec4(pixelX, pixelY, -1, 1);
      			glm::vec4 worldFar  = glm::inverse(viewMatrix) * glm::inverse(projMatrix) * glm::vec4(pixelX, pixelY,  1, 1);
      		
      			glm::vec3 worldNearPos = glm::vec3(worldNear.x, worldNear.y, worldNear.z) / worldNear.w;
      			glm::vec3 worldFarPos  = glm::vec3(worldFar.x, worldFar.y, worldFar.z) / worldFar.w;
      
			Payload payload;
			IntersectInfo info;
			Ray ray(worldNearPos, glm::normalize(glm::vec3(worldFarPos - worldNearPos)));	

			if (CheckIntersection(ray, info)) {
				payload.color += checkLight(info);
				glm::vec3 color = CastRay(ray,payload, info);
				glColor3f(color.x,color.y,color.z);
			}
			else
				glColor3f(0.f, 0.f, 0.f);
			
			glVertex3f(x, y, 0.0f);
		}
  	
	glEnd();
	glFlush();
	glutSwapBuffers();
	std::cout << "Done " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 100) / 100 << " s" << std::endl;

}


int main(int argc, char **argv) {
#pragma region Create Scene
	/*
	* Creates the floor with chess pattern
	*/
	bool color_flag = true;
	Material mat;
	for (int i = 0; i > -11; i--)
		for (int z = 0; z < 11; z++){
			mat = color_flag ? white : black;
			objects.push_back(new Plane(glm::vec3(i, 0.f, z),		// point 1
				glm::vec3(i - 1, 0.f, z),				// point 2
				glm::vec3(i - 1, 0.f, z + 1),				// point 3
				glm::vec3(i, 0.f, z + 1),				// point 4  
				mat)								// Material
				);
			color_flag = !color_flag;
		}
	/**/

	/*
	* Creates the floor overlay for shadow casting
	*/
	Plane floor(glm::vec3(0.f, 0.f, 0.f),		// point 1
		glm::vec3(-11.f, 0.f, 0.f),				// point 2
		glm::vec3(-11.f, 0.f, 11.f),			// point 3
		glm::vec3(0.f, 0.f, 11.f),				// point 4
		mat										// material
		);
	can_cast_shadow.push_back(&floor);
	/**/

	/*
	* Creates the front right wall (mirror)
	*/
	Plane front_right_wall(glm::vec3(0.f, 0.f, 0.f),		// point 1
		glm::vec3(0.f, 0.f, 11.f),				// point 2
		glm::vec3(0.f, 11.f, 11.f),			// point 3
		glm::vec3(0.f, 11.f, 0.f),				// point 4
		mirror								// material
		);
	objects.push_back(&front_right_wall);
	can_cast_shadow.push_back(&front_right_wall);
	/**/

	/*
	* Creates the front left wall (mirror)
	*/
	Plane front_left_wall(glm::vec3(0.f, 0.f, 0.f),		// point 1
		glm::vec3(0.f, 11.f, 0.f),				// point 2
		glm::vec3(-11.f, 11.f, 0.f),			// point 3
		glm::vec3(-11.f, 0.f, 0.f),				// point 4
		mirror								// material
		);
	objects.push_back(&front_left_wall);
	can_cast_shadow.push_back(&front_left_wall);
	/**/

	/*
	* Creates a sphere
	*/
	Sphere ball(glm::vec3(-2.f, 1.f, 2.f),		  	// position
		1.f,								// radius
		Material(glm::vec3(0.f, .2f, .2f),  // ambient 
		glm::vec3(.3f, .5f, .5f),		// diffuse
		glm::vec3(1.f, 1.f, 1.f),		// specular
		10.f,						// glossiness
		.0f,							// reflectivity			
		1.5f)						// refractivity			
		);
	objects.push_back(&ball);
	can_cast_shadow.push_back(&ball);
	/**/

	/*
	* Creates a triangle
	*/
	Triangle trigwno(glm::vec3(-5.f, 0.f, 1.f),	// point 1
		glm::vec3(-4.f, 0.f, 3.f),			// point 2
		glm::vec3(-4.f, 3.f, 2.f),			// point 3
		Material(glm::vec3(.2f, .2f, .2f),	// ambient 
		glm::vec3(.5f, .5f, .0f),  			// diffuse
		glm::vec3(1.f, 1.f, 1.f), 			// specular
		3.f,								// glossiness
		.0f,								// reflectivity
		.0f)								// refractivity
		);
	objects.push_back(&trigwno);
	can_cast_shadow.push_back(&trigwno);
	/**/
#pragma endregion

#pragma region OpenGL Parameters
	glutInit(&argc, argv);
	glutInitWindowSize(windowX, windowY);	
	glutCreateWindow("RayTracer");	
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, windowX, windowY, 0, -512, 512);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);

#pragma endregion

#pragma region Thread Execution	
	initialise_thread_variables();
	glutDisplayFunc(render_threads);
#pragma endregion

#pragma region Normal Execution
//	glutDisplayFunc(Render);
#pragma endregion

	atexit(cleanup);
	glutMainLoop();
	return 0;
}
