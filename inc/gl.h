
#pragma once

#include "glcommon.h"
#include "common.h"

namespace ogl {
	struct opengl {
		opengl() {

			assert(glfwInit() && "GLFW Initialization Error");

			glfwWindowHint(GLFW_SAMPLES, 4); // 4x anti-aliasing
			glfwWindowHint(GLFW_VERSION_MAJOR, 3); // OpenGL 3.3
			glfwWindowHint(GLEW_VERSION_MINOR, 2);
			glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);


			//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
			//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // No old OpenGL
		}

		~opengl() {
			glfwTerminate();
		}
	};
};