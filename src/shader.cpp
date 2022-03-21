
#include "glcommon.h"

#include "strings.h"
#include "array.h"

#include "array.h"
#include "osystem.h"
#include "..\inc\shader.h"

using namespace ogl;

string shader_path = "../rsc/shaders/";

bool shader::compile_shader(const char* ShaderCode, GLuint ShaderID) {
	GLint Result = GL_FALSE;
	int InfoLogLength;

	char const* SourcePointer = ShaderCode;
	glShaderSource(ShaderID, 1, &SourcePointer, NULL);
	glCompileShader(ShaderID);

	// Check Shader
	glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);

	if (InfoLogLength > 0) {
		Array<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(ShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	return Result;
}


void shader::load(const char* pvert, const char* pgeom, const char* pfrag) {


	// Create the shaders
	VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	if (pgeom) GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);

	GLint Result = GL_FALSE;
	int InfoLogLength;

	std::string vert = std::string(shader_path.cstr()) + pvert + ".vert";
	std::string geom = GeometryShaderID ? std::string(shader_path.cstr()) + pgeom + ".geom" : "";
	std::string frag = std::string(shader_path.cstr()) + pfrag + ".frag";

	printf("Compiling shader : %s\n", vert.c_str());
	string tmp = read_file(vert.c_str());
	compile_shader(tmp.get_writable(), VertexShaderID);

	if (GeometryShaderID) {
		// Compile Geometry Shader
		printf("Compiling shader : %s\n", geom.c_str());
		compile_shader(read_file(geom.c_str()).get_writable(), GeometryShaderID);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", frag.c_str());
	compile_shader(read_file(frag.c_str()).get_writable(), FragmentShaderID);

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	if (GeometryShaderID) glAttachShader(ProgramID, GeometryShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);

	if (InfoLogLength > 0) {
		Array<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);
	if (GeometryShaderID) glDetachShader(ProgramID, GeometryShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);
	if (GeometryShaderID) glDeleteShader(GeometryShaderID);

	programm = ProgramID;
}

shader::shader(const char* vert, const char* geom, const char* frag) {
	load(vert, geom, frag);
}

void shader::bind() {
	glUseProgram(programm);
}

GLuint ogl::shader::getu(const char* uid) {
	return glGetUniformLocation(programm, uid);
}

void shader::unbind() {
	glUseProgram(0);
}

shader::~shader() {
	glDeleteProgram(programm);
}
