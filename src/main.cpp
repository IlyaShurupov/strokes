
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
MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}

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

struct ogl_window {
	GLFWwindow* window;
	glm::vec4 clear_col = glm::vec4(0, 0, 0, 0.1);
	glm::vec2 size = glm::vec2(1024, 768);

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


		glEnable(GL_ALPHA_TEST);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		//glEnable(GL_CULL_FACE);

		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(MessageCallback, 0);

		//glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
	}

	inline void mod_win() {
		glfwMakeContextCurrent(window); // Initialize GLEW
	}

	void begin_draw() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, size.x, size.y); // Render on the whole framebuffer, complete from the lower left corner to the upper right
		// Clear the screen
		glClearColor(clear_col.r, clear_col.g, clear_col.b, clear_col.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	bool Close() {
		return (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
	}

	void end_draw() {
		glfwSwapBuffers(window);
		glfwPollEvents();
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



struct ogl_frame_buffer {

	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };

	GLuint FramebufferName;

	GLuint renderedTexture;
	GLuint depthrenderbuffer;

	glm::vec2 size;

	glm::vec4 clear_col = glm::vec4(0, 0, 0, 1);

	ogl_frame_buffer(glm::vec2 p_size) {

		size = p_size;

		// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
		FramebufferName = 0;
		glGenFramebuffers(1, &FramebufferName);
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

		// The texture we're going to render to
		renderedTexture;
		glGenTextures(1, &renderedTexture);

		// "Bind" the newly created texture : all future texture functions will modify this texture
		glBindTexture(GL_TEXTURE_2D, renderedTexture);

		// Give an empty image to OpenGL ( the last "0" )
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		// Poor filtering. Needed !
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		// The depth buffer
		glGenRenderbuffers(1, &depthrenderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size.x, size.y);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

		// Set "renderedTexture" as our colour attachement #0
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

		// Set the list of draw buffers.
		//GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

		// Always check that our framebuffer is ok
		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	}

	void begin_draw() {
		// Render to our framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
		glViewport(0, 0, size.x, size.y); // Render on the whole framebuffer, complete from the lower left corner to the upper right
		glClearColor(clear_col.r, clear_col.g, clear_col.b, clear_col.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(0);
	}

	void end_draw() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);
	}


	void test_draw() {

	}

	~ogl_frame_buffer() {
		glDeleteFramebuffers(1, &FramebufferName);
		glDeleteTextures(1, &renderedTexture);
		glDeleteRenderbuffers(1, &depthrenderbuffer);
	}
};

struct ogl_texture_drawer {

	GLuint quad_VertexArrayID;
	GLuint quad_vertexbuffer;
	GLuint quad_programID;
	GLuint texID;
	GLuint timeID;
	GLuint rect_mat;

	glm::vec4 clear_col = glm::vec4(0, 1, 0, 1);

	const GLfloat g_quad_vertex_buffer_data[18] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,
	};

	ogl_texture_drawer() {
		// The fullscreen quad's FBO
		glGenVertexArrays(1, &quad_VertexArrayID);
		glBindVertexArray(quad_VertexArrayID);

		glGenBuffers(1, &quad_vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

		// Create and compile our GLSL program from the shaders
		quad_programID = LoadShaders("../rsc/shaders/Passthrough.vert", "../rsc/shaders/Texture.frag");
		texID = glGetUniformLocation(quad_programID, "renderedTexture");
		timeID = glGetUniformLocation(quad_programID, "time");
		rect_mat = glGetUniformLocation(quad_programID, "RectMat");
	}

	glm::mat4 get_rect_transform_mat(const glm::vec4& target, const glm::vec2& domen_size) {

		float scale_x = (target.z - target.x) / domen_size.x;
		float scale_y = (target.w - target.y) / domen_size.y;

		float move_x = (target.x / domen_size.x) - (1 - scale_x);
		float move_y = (target.y / domen_size.y) - (1 - scale_y);

		glm::mat4 out = glm::mat4(1.f);

		out[3][0] = move_x;
		out[3][1] = move_y;

		out[0][0] = scale_x;
		out[1][1] = scale_y;

		return out;
	}

	void draw_texture(ogl_frame_buffer* to_tex, ogl_frame_buffer* from, glm::vec4 rec) {

		assert(from);

		// Render to the screen
		glBindFramebuffer(GL_FRAMEBUFFER, to_tex ? to_tex->renderedTexture : 0);

		if (to_tex) {
			glViewport(0, 0, from->size.x, from->size.y); // Render on the whole framebuffer, complete from the lower left corner to the upper right
			// Clear the screen
			//glClearColor(clear_col.r, clear_col.g, clear_col.b, clear_col.a);
			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		// Use our shader
		glUseProgram(quad_programID);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, from->renderedTexture);
		// Set our "renderedTexture" sampler to use Texture Unit 0
		glUniform1i(texID, 0);

		glm::mat4 tmat = to_tex ? get_rect_transform_mat(rec, to_tex->size) : glm::mat4(1.f);

		glUniformMatrix4fv(rect_mat, 1, GL_FALSE, &tmat[0][0]);

		glUniform1f(timeID, (float)(glfwGetTime() * 10.0f));

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// Draw the triangles !
		glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles

		glDisableVertexAttribArray(0);
	}

	~ogl_texture_drawer() {
		glDeleteBuffers(1, &quad_vertexbuffer);
		glDeleteProgram(quad_programID);
		glDeleteVertexArrays(1, &quad_VertexArrayID);
	}
};



struct ogl_vertex_data {
	GLfloat X;
	GLfloat Y;
	GLfloat Z;

	void set(glm::vec3 pos) {
		X = pos.x;
		Y = pos.y;
		Z = pos.z;
	}
};

struct ogl_color_data {
	GLfloat X = 1;
	GLfloat Y = 1;
	GLfloat Z = 1;
};

struct ogl_camera {

	glm::vec3 pos;
	glm::vec3 target;

	alnf fov;
	alnf near;
	alnf far;

	bool orto;

	float ratio;

	ogl_camera() {
		reset();
	}

	void reset() {
		pos = glm::vec3(0, 0, 4);
		target = glm::vec3(0, 0, 0);

		fov = 45.f;
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

		glm::vec<2, double, glm::defaultp> mousepos;
		glfwGetCursorPos(window, &mousepos.x, &mousepos.y);

		static glm::vec<2, double, glm::defaultp> prev_pos = mousepos;

		glm::vec2 delta = prev_pos - mousepos;
		float degree_X = delta.x;
		float degree_Y = delta.y;

		// Move forward
		glm::vec3 forward = glm::normalize(target - pos);
		glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
		glm::vec3 up = glm::normalize(glm::cross(right, forward));

		glm::vec3 rot_axis;

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_3) == GLFW_PRESS) {
			glm::mat4 rot_mat = glm::rotate(glm::mat4(1.0f), glm::radians(-degree_Y), right);
			pos = glm::vec3(glm::vec4(pos, 1.0f) * rot_mat);

			rot_mat = glm::rotate(glm::mat4(1.0f), glm::radians(-degree_X), glm::vec3(0, 1, 0));
			pos = glm::vec3(glm::vec4(pos, 1.0f) * rot_mat);
		}

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
			pos += delta.y * (forward * glm::vec3(0.03));
		}

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			glm::vec3 move = right * glm::vec3(0.03 * delta.x) + up * glm::vec3(0.03 * -delta.y);
			pos += move;
			target += move;
		}

		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
			reset();
		}

		prev_pos = mousepos;
	}
};

struct vertex_barycentric {
	GLfloat val[3] = { 0, 0, 0 };

	void set(glm::vec3 in) {
		val[0] = in.x;
		val[1] = in.y;
		val[2] = in.z;
	}
};

struct ogl_mesh {

	Array<ogl_vertex_data> g_vertex_buffer_data;
	Array<uint4> g_trig_idxs;

	Array<ogl_color_data> g_color_buffer_data;

	Array<vertex_barycentric> g_vertex_barycentric;

	glm::mat4 MTXmodel = glm::mat4(1.f);

	GLuint programID;
	GLuint MatrixID;
	GLuint VertexArrayID;
	GLuint vertexbuffer;
	GLuint colorbuffer;
	GLuint elementbuffer;
	GLuint barycentricbuffer;

	ogl_mesh() {
		glUseProgram(0);
		programID = LoadShaders("../rsc/shaders/default.vert", "../rsc/shaders/default.frag");
		glUseProgram(programID);
		MatrixID = glGetUniformLocation(programID, "MVP");

		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		glGenBuffers(1, &vertexbuffer);

		glGenBuffers(1, &colorbuffer);

		glGenBuffers(1, &elementbuffer);

		glGenBuffers(1, &barycentricbuffer);
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

		g_vertex_barycentric.Reserve(mesh.Vertices.size());
		for (alni i = 0; i < mesh.Vertices.size(); i++) {
			g_vertex_barycentric[i].val[i % 3] = 1.f;
		}
	}

	void bind_buffers() {
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data[0]) * g_vertex_buffer_data.length, g_vertex_buffer_data.buff, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data[0]) * g_color_buffer_data.length, g_color_buffer_data.buff, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_trig_idxs[0]) * g_trig_idxs.length, g_trig_idxs.buff, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, barycentricbuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_vertex_barycentric[0]) * g_vertex_barycentric.length, g_vertex_barycentric.buff, GL_STATIC_DRAW);
	}

	glm::mat4 get_MPV(ogl_camera& cam) {
		return cam.get_cam_proj_mat() * cam.get_cam_view_mat() * MTXmodel;
	}

	void draw_mesh(ogl_camera* cam) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		glUseProgram(programID);
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &get_MPV(*cam)[0][0]);

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

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, barycentricbuffer);
		glVertexAttribPointer(
			2,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);

		// Draw the triangles ! // mode. count. type. element. array buffer offset
		glDrawElements(GL_TRIANGLES, g_trig_idxs.length, GL_UNSIGNED_INT, (void*)0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
	}

	~ogl_mesh() {
		glDeleteBuffers(1, &vertexbuffer);
		glDeleteBuffers(1, &elementbuffer);
		glDeleteBuffers(1, &barycentricbuffer);
		glDeleteProgram(programID);
		glDeleteVertexArrays(1, &VertexArrayID);
	}
};


struct stroke_point {
	glm::vec3 pos = glm::vec3(0, 0, 0);
	glm::vec4 col = glm::vec4(1, 1, 1, 1);
	glm::vec3 normal = glm::normalize(glm::vec3(0, randf(), 0));
	float thikness = 0.06;

	stroke_point() {
	}

	stroke_point(glm::vec3 p_pos, glm::vec3 norm, float pthickness = 0.06) {
		pos = p_pos;
		normal = norm;
		thikness = pthickness;
	}
};

struct stroke {
	Array<stroke_point> points;
};

struct Painter {

	enum class pstate {
		NONE,
		ACTIVE,
	} state = pstate::NONE;

	stroke input;
	float precision = 0.01;

	void add_point(const glm::vec3& pos, const glm::vec3& norm, float thickness = 0.06) {
		input.points.PushBack(stroke_point(pos, norm, thickness));
	}

	glm::vec3 project_3d(const glm::vec2& cpos, ogl_camera* cam) {
		glm::vec3 forw = glm::normalize(cam->target - cam->pos);
		glm::vec3 right = glm::normalize(glm::cross(forw, glm::vec3(0, 1, 0)));
		glm::vec3 up = glm::cross(right, forw);
		
		float scale = glm::tan(glm::radians(cam->fov)/2) * glm::length(cam->target - cam->pos);
		//float b = 1 / ;
		//float a = 1 / b;
		glm::vec3 out = cam->target + (cpos.x * scale * right * cam->ratio) - (cpos.y * scale * up );
		return out;
	}

	bool passed(const glm::vec2& cur, const glm::vec2& prevcur) {
		return glm::length(prevcur - cur) > precision;
	}

	void start(const glm::vec2& cpos, ogl_camera* cam) {
		input.points.Free();
		glm::vec3 point = project_3d(cpos, cam);
		add_point(point, glm::normalize(cam->target - cam->pos), 0.01);
	}

	void sample(const glm::vec2& cpos, ogl_camera* cam) {
		glm::vec3 point = project_3d(cpos, cam);
		add_point(point, glm::normalize(cam->target - cam->pos));
	}

	void finish(const glm::vec2& cpos, ogl_camera* cam) {
		if (input.points.length == 1) {
			input.points.Free();
		}
		else {
			input.points[input.points.length - 1].thikness = 0.01;
		}
	}

	// cpos - normilized coordinates from center
	void update(glm::vec2 curs, bool mouse_down, ogl_camera* cam) {

		static glm::vec2 prev_curs = curs;

		switch (state) {
			case pstate::NONE: {
				if (mouse_down) {
					start(curs, cam);
					state = pstate::ACTIVE;
					prev_curs = curs;
				}
				return;
			}
			case pstate::ACTIVE: {
				if (!mouse_down) {
					finish(curs, cam);
					state = pstate::NONE;
					return;
				}
				if (passed(curs, prev_curs)) {
					sample(curs, cam);
					prev_curs = curs;
				}
				return;
			}
		}
	}
};

struct Drawing {

	list<stroke> strokes;

	void gen_quad(alni vidx, ogl_mesh& out, stroke_point* p1, stroke_point* p2, glm::vec3 dir1, glm::vec3 dir2) {
		glm::vec3 perp = glm::normalize(glm::cross(p1->normal, p2->pos - p1->pos)) * p1->thikness;
		float cosa1 = glm::dot(perp, dir1) / (glm::length(dir1) * glm::length(perp));
		float cosa2 = glm::dot(perp, dir2) / (glm::length(dir2) * glm::length(perp));
		//float thickness1 = -p1->thikness / cosa1;
		//float thickness2 = -p2->thikness / cosa2;

		dir1 = glm::normalize(dir1);
		dir2 = glm::normalize(dir2);

		glm::vec3 v1 = p1->pos + dir1 * p1->thikness;
		glm::vec3 v2 = p2->pos + dir2 * p2->thikness;

		glm::vec3 v3 = p2->pos - dir2 * p2->thikness;
		glm::vec3 v4 = p1->pos - dir1 * p1->thikness;


		out.g_vertex_buffer_data[vidx].set(v1);
		out.g_vertex_buffer_data[vidx + 1].set(v2);
		out.g_vertex_buffer_data[vidx + 2].set(v3);
		out.g_vertex_buffer_data[vidx + 3].set(v3);
		out.g_vertex_buffer_data[vidx + 4].set(v4);
		out.g_vertex_buffer_data[vidx + 5].set(v1);

		out.g_trig_idxs[vidx] = vidx;
		out.g_trig_idxs[vidx + 1] = vidx + 1;
		out.g_trig_idxs[vidx + 2] = vidx + 2;
		out.g_trig_idxs[vidx + 3] = vidx + 3;
		out.g_trig_idxs[vidx + 4] = vidx + 4;
		out.g_trig_idxs[vidx + 5] = vidx + 5;

		out.g_vertex_barycentric[vidx].set(glm::vec3(1, 0, 0));
		out.g_vertex_barycentric[vidx + 1].set(glm::vec3(0, 1, 0));
		out.g_vertex_barycentric[vidx + 2].set(glm::vec3(0, 0, 1));
		out.g_vertex_barycentric[vidx + 3].set(glm::vec3(1, 0, 0));
		out.g_vertex_barycentric[vidx + 4].set(glm::vec3(0, 1, 0));
		out.g_vertex_barycentric[vidx + 5].set(glm::vec3(0, 0, 1));
	}


	glm::vec3 get_dir_vec(glm::vec3 v1, glm::vec3 v2, const glm::vec3& norm) {
		
		v1 = glm::normalize(v1);
		v2 = glm::normalize(v2);

		glm::vec3 plane_normal;

		float val = glm::dot(v2, v1);
		if (val < -0.999999) {
			plane_normal = v2;
		}
		else {
			glm::vec3 cross = glm::cross(v1, v2);
			float rot_angle = glm::acos(glm::dot(v1, v2)) / 2.f;
			glm::mat4 rot_matrix = glm::rotate(rot_angle, cross);
			glm::vec3 middle = rot_matrix * glm::vec4(v1.x, v1.y, v1.z, 0);
			plane_normal = glm::cross(middle, cross);
		}
		
		return glm::cross(plane_normal, norm);
	}

	void solve_stroke(alni& vidx, ogl_mesh& out, stroke* str) {
		alni len = str->points.length;
		for (alni pidx = 0; pidx < len - 1; pidx++) {

			stroke_point pt0;
			stroke_point pt1 = str->points[pidx];
			stroke_point pt2 = str->points[pidx + 1];
			stroke_point pt3;

			glm::vec3 dir1;
			glm::vec3 dir2;

			if (pidx > 0) {
				pt0 = str->points[pidx - 1];
			}
			else {
				pt0.pos = pt1.pos + (pt1.pos - pt2.pos);
				pt0.thikness = 0.001;
			}

			if (pidx < len - 2) {
				pt3 = str->points[pidx + 2];
			}
			else {
				pt3.pos = pt2.pos + (pt2.pos - pt1.pos);
				pt3.thikness = 0.001;
			}

			dir1 = get_dir_vec(pt0.pos - pt1.pos, pt2.pos - pt1.pos, pt1.normal);
			dir2 = get_dir_vec(pt1.pos - pt2.pos, pt3.pos - pt2.pos, pt2.normal);

			if (glm::dot(dir1, dir2) < 0) {
				dir1 *= -1;
			}

			gen_quad(vidx * 6, out, &pt1, &pt2, dir1, dir2);

			vidx++;
		}
	}

	void to_mesh(ogl_mesh& out, Painter* painter) {

		alni mesh_vertex_len = 0;
		for (auto str : strokes) {
			mesh_vertex_len += (str.Data().points.length - 1) * 6;
		}
		if (painter->input.points.length) mesh_vertex_len += (painter->input.points.length - 1) * 6;

		out.g_vertex_buffer_data.Reserve(mesh_vertex_len);
		out.g_color_buffer_data.Reserve(mesh_vertex_len);
		out.g_vertex_barycentric.Reserve(mesh_vertex_len);
		out.g_trig_idxs.Reserve(mesh_vertex_len);

		alni vidx = 0;
		for (auto str : strokes) {
			solve_stroke(vidx, out, &str.Data());
		}

		if (painter->input.points.length > 1) {
			solve_stroke(vidx, out, &painter->input);
			if (painter->state == Painter::pstate::NONE) {
				strokes.PushBack(painter->input);
				painter->input.points.Free();
			}
		}
	}
};

int main() {

	opengl ogl;
	ogl_window window;

	ogl_mesh mesh;
	ogl_camera cam;
	mesh.load("../rsc/cube.obj");

	mesh.bind_buffers();

	ogl_frame_buffer fbo(glm::vec2(1024, 768)); fbo.clear_col = glm::vec4(0, 0, 0, 0);
	ogl_texture_drawer tex_draw;

	double lastTime = glfwGetTime();
	int nbFrames = 0;

	ogl_frame_buffer fbo2(glm::vec2(500, 500 * 3 / 4.f));


	stroke_point p1;
	p1.pos = glm::vec3(-1, 0, 0);

	stroke_point p2;
	p2.pos = glm::vec3(1, 0, 0);

	stroke st;
	st.points.PushBack(p1);
	st.points.PushBack(p2);

	Drawing dr;
	dr.strokes.PushBack(st);

	ogl_mesh strokes_mesh;
	Painter painter;

	do {

		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1 sec ago
				// printf and reset timer
			printf("%f ms/frame FPS: %f \n", 1000.0 / double(nbFrames), double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		cam.update(window.window);

		glm::vec<2, double, glm::defaultp> mousepos;
		glfwGetCursorPos(window.window, &mousepos.x, &mousepos.y);

		bool draw = glfwGetMouseButton(window.window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS;
		painter.update(glm::vec2((mousepos.x / window.size.x - 0.5)*2, (mousepos.y / window.size.y - 0.5)*2), draw, &cam);

		dr.to_mesh(strokes_mesh, &painter);
		strokes_mesh.bind_buffers();

		fbo.begin_draw(); {
			strokes_mesh.draw_mesh(&cam);
		} fbo.end_draw();

		fbo2.begin_draw(); {

			mesh.draw_mesh(&cam);

			/*
			glColor3f(1, 1, 1);

			// Begin the pointer
			glBegin(GL_POLYGON);

			// Iterate through all the
			// 360 degrees
			for (int i = 0; i < 360; i++) {
				glVertex2f(cos(glm::radians((float)i)) * 0.4, sin(glm::radians((float)i)) * 0.4);
			}

			// Sets vertex
			glEnd();
			*/

		} fbo2.end_draw();


		tex_draw.draw_texture(&fbo, &fbo2, glm::vec4(100, 100, 500, 3.f / 4 * 500));

		window.begin_draw();

		tex_draw.draw_texture(0, &fbo, glm::vec4(200, 200, 500, 3.f / 4 * 500));

		window.end_draw();

	} while (window.Close());

	return 0;
}