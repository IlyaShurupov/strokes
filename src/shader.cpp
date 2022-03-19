
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

	const char* vert = (shader_path + string(pvert) + ".vert").cstr();
	const char* geom = GeometryShaderID ? (shader_path + string(pgeom) + ".geom").cstr() : NULL;
	const char* frag = (shader_path + string(pfrag) + ".frag").cstr();

	printf("Compiling shader : %s\n", vert);
	compile_shader(read_file(vert).cstr(), VertexShaderID);

	if (GeometryShaderID) {
		// Compile Geometry Shader
		printf("Compiling shader : %s\n", geom);
		compile_shader(read_file(geom).cstr(), GeometryShaderID);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", frag);
	compile_shader(read_file(frag).cstr(), FragmentShaderID);

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
