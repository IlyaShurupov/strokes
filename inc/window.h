
#pragma once

#include "glcommon.h"

namespace ogl {

	class window {
	public:

		GLFWwindow* winp = NULL;
		vec4 col_clear;
		vec2 size;

		void resize(vec2 psize);
		void init();

		window();
		window(vec2 size);
		
		void set_current();
		
		void begin_draw();
		
		void clear();
		
		void end_draw();

		bool CloseSignal();

		GLFWwindow* geth();

		vec2 cursor(bool normalized = false);

		float pen_pressure();

		bool rmb();

		~window();
	};

};