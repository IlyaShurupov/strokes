
#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;

inline vec3 cross(const vec3& in1, const vec3& in2) {
	return glm::cross(in1, in2);
};

inline vec3 normalize(const vec3& in) {
	return glm::normalize(in);
};

inline double radians(double in) {
	return glm::radians(in);
}

struct frame_counter {

	double lastTime = glfwGetTime();
	int nbFrames = 0;

	void log(int& val) {
		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		// If last prinf() was more than 1 sec ago
		if (currentTime - lastTime >= 1.0) {
			// printf and reset timer
			val = nbFrames;
			printf("%f ms/frame FPS: %f \n", 1000.0 / double(nbFrames), double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}
	}
};