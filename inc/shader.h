
#pragma once

namespace ogl {

	class shader {

		typedef unsigned int GLuint;

		GLuint programm;
		GLuint VertexShaderID;
		GLuint FragmentShaderID;
		GLuint GeometryShaderID = 0;

		bool compile_shader(const char* ShaderCode, GLuint ShaderID);
		void load(const char* vert, const char* geom, const char* frag);

	public:

		shader(const char* vert, const char* geom, const char* frag);
		void bind();
		GLuint getu(const char* uid);
		void unbind();
		~shader();
	};

};
