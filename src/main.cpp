
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "objloader.h"

#include "allocators.h"
#include "containers.h"
#include "strings.h"
#include "osystem.h"
#include "common.h"

void GLAPIENTRY
MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}


struct opengl {
	opengl() {

		assert(glfwInit() && "GLFW Initialization Error");

		glfwWindowHint(GLFW_SAMPLES, 4); // 4x anti-aliasing
		glfwWindowHint(GLFW_VERSION_MAJOR, 3); // OpenGL 3.3
		glfwWindowHint(GLEW_VERSION_MINOR, 2);

		//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
		//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // No old OpenGL
	}

	~opengl() {
		glfwTerminate();
	}
};

struct ogl_window {
	GLFWwindow* window;

	ogl_window() {
		window = glfwCreateWindow(1024, 768, "Tutorial 01", NULL, NULL);
		glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

		if (window == NULL) {
			fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
			glfwTerminate();
			assert(0);
		}

		glfwMakeContextCurrent(window);
		//glewExperimental = true; // Needed in core profile
		if (glewInit() != GLEW_OK) {
			fprintf(stderr, "Failed to initialize GLEW\n");
			assert(0);
		}

		glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);

		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(MessageCallback, 0);
	}

	inline void mod_win() {
		glfwMakeContextCurrent(window); // Initialize GLEW
	}

	void begin_draw() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	bool end_draw() {
		glfwSwapBuffers(window);
		glfwPollEvents();
		return (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
	}

	~ogl_window() {
		glfwDestroyWindow(window);
	}
};


GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	string VertexShaderCode = read_file(vertex_file_path);

	// Read the Fragment Shader code from the file
	string FragmentShaderCode = read_file(fragment_file_path);


	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.cstr();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);

	if (InfoLogLength > 0) {
		Array<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.cstr();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		Array<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
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

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

struct ogl_camera {

	glm::vec3 pos;
	glm::vec3 target;

	alnf fov;
	alnf near;
	alnf far;

	bool orto;

	alnf ratio;

	ogl_camera() {
		reset();
	}

	void reset() {
		pos = glm::vec3(0, 0, 4);
		target = glm::vec3(0, 0, 0);

		fov = 100.f;
		near = 0.01f;
		far = 100.f;

		orto = false;

		ratio = 4 / 3.f;
	}

	glm::mat4 get_cam_proj_mat() {
		return  glm::perspective(glm::radians((float)fov), (float)ratio, (float)near, (float)far);
	}

	glm::mat4 get_cam_view_mat() {
		return glm::lookAt(pos, target, glm::vec3(0, 1, 0));
	}

	void update(GLFWwindow* window) {
		// Move forward
		glm::vec3 forward = glm::normalize(target - pos);
		glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
		glm::vec3 up = glm::normalize(glm::cross(right, forward));

		float degree = 0.2;
		glm::vec3 rot_axis;

		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
			glm::mat4 rot_mat = glm::rotate(glm::mat4(1.0f), glm::radians(degree), right);
			pos = glm::vec3(glm::vec4(pos, 1.0f) * rot_mat);
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
			glm::mat4 rot_mat = glm::rotate(glm::mat4(1.0f), glm::radians(-degree), right);
			pos = glm::vec3(glm::vec4(pos, 1.0f) * rot_mat);
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			glm::mat4 rot_mat = glm::rotate(glm::mat4(1.0f), glm::radians(degree), glm::vec3(0, 1, 0));
			pos = glm::vec3(glm::vec4(pos, 1.0f) * rot_mat);
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
			glm::mat4 rot_mat = glm::rotate(glm::mat4(1.0f), glm::radians(-degree), glm::vec3(0, 1, 0));
			pos = glm::vec3(glm::vec4(pos, 1.0f) * rot_mat);
		}

		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
			pos += forward * glm::vec3(0.01);
		}
		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
			pos -= forward * glm::vec3(0.01);
		}

		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
			reset();
		}
	}
};

struct ogl_vertex_data {
	GLfloat X;
	GLfloat Y;
	GLfloat Z;
};

struct ogl_color_data {
	GLfloat X = 1;
	GLfloat Y = 1;
	GLfloat Z = 1;
};

struct ogl_mesh {

	Array<ogl_vertex_data> g_vertex_buffer_data;
	Array<uint4> g_trig_idxs;

	Array<ogl_color_data> g_color_buffer_data;

	glm::mat4 MTXmodel = glm::mat4(1.f);

	ogl_mesh() {
	}

	void load(string path) {
		objl::Loader loader;
		loader.LoadFile(path.cstr());

		objl::Mesh& mesh = loader.LoadedMeshes[0];

		g_vertex_buffer_data.Reserve(mesh.Vertices.size());
		for (alni i = 0; i < loader.LoadedMeshes[0].Vertices.size(); i++) {
			g_vertex_buffer_data[i].X = mesh.Vertices[i].Position.X;
			g_vertex_buffer_data[i].Y = mesh.Vertices[i].Position.Y;
			g_vertex_buffer_data[i].Z = mesh.Vertices[i].Position.Z;
		}

		g_trig_idxs.Reserve(mesh.Indices.size());
		for (alni i = 0; i < mesh.Indices.size(); i++) {
			g_trig_idxs[i] = mesh.Indices[i];
		}

		g_color_buffer_data.Reserve(mesh.Vertices.size());
		for (alni i = 0; i < mesh.Vertices.size(); i++) {
			g_color_buffer_data[i].X = randf();
			g_color_buffer_data[i].Y = randf();
			g_color_buffer_data[i].Z = randf();
		}

	}

	glm::mat4 get_MPV(ogl_camera& cam) {
		return cam.get_cam_proj_mat() * cam.get_cam_view_mat() * MTXmodel;
	}
};

int main() {

	opengl ogl;
	ogl_window window;
	ogl_mesh mesh;
	ogl_camera cam;

	mesh.load("../rsc/cube.obj");

	GLuint programID = LoadShaders("../rsc/shaders/default.vert", "../rsc/shaders/default.frag");
	glUseProgram(programID);
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);

	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);

	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);


	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mesh.g_vertex_buffer_data[0]) * mesh.g_vertex_buffer_data.length, mesh.g_vertex_buffer_data.buff, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mesh.g_color_buffer_data[0]) * mesh.g_color_buffer_data.length, mesh.g_color_buffer_data.buff, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(mesh.g_trig_idxs[0]) * mesh.g_trig_idxs.length, mesh.g_trig_idxs.buff, GL_STATIC_DRAW);

	do {
		window.begin_draw();

		cam.update(window.window);

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mesh.get_MPV(cam)[0][0]);

		// 1st attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glVertexAttribPointer(
			1,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);

		/*
		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
		glDisableVertexAttribArray(0);
		*/

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
		
		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,      // mode
			mesh.g_trig_idxs.length,    // count
			GL_UNSIGNED_INT,   // type
			(void*)0           // element array buffer offset
		);

	} while (window.end_draw());
}