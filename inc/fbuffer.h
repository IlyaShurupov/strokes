
#pragma once

#include "glcommon.h"

namespace ogl {

	class fbuffer {

		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		GLuint FramebufferName;
		GLuint renderedTexture;
		GLuint depthrenderbuffer;

		glm::vec2 size;
		glm::vec4 col_clear;

	public:

		fbuffer(vec2 p_size, vec4 pcol_clear);
		void begin_draw();
		void clear();
		void end_draw();
		GLuint texId();
		~fbuffer();
	};

};