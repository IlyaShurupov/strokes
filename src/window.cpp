
#include "window.h"

#include "glutils.h"

void GLAPIENTRY
MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}

using namespace ogl;

void window::resize(vec2 psize) {
	size = psize;
	if (winp) {
		// apply
	}
}

window::window() {
	col_clear = vec4(0, 0, 0, 0.1);
	size = vec2(1024, 768);
	init();
}

namespace WIN {
#include <Windows.h>
};


void window::init() {
	resize(size);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();

	//glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
	
	winp = glfwCreateWindow(size.x, size.y, "NULL", NULL, NULL);

	if (winp == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
	}

	glfwSetWindowPos(winp, 0, 0);

	glfwSetInputMode(winp, GLFW_STICKY_KEYS, GL_TRUE);

	// wtf
	glfwMakeContextCurrent(winp);
	//glewExperimental = true; // Needed in core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
	}

	WIN::HDC ourWindowHandleToDeviceContext = WIN::GetDC(WIN::GetActiveWindow());
	WIN::MessageBoxA(0, (char*)glGetString(GL_VERSION), "OPENGL VERSION", 0);

	init_utils();

	glEnable(GL_ALPHA_TEST);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	//glEnable(GL_CULL_FACE);

	glEnable(GL_DEBUG_OUTPUT);

	glfwSetWindowAttrib(winp, GLFW_DECORATED, GLFW_FALSE);

	glDebugMessageCallback(MessageCallback, 0);

	int x, y;
	glfwGetWindowSize(winp, &x, &y);
	size.x = x;
	size.y = y;
}

window::window(vec2 psize) {
	size = psize;
	init();
}

void window::begin_draw() {
	// Render on the whole framebuffer, complete from the lower left corner to the upper right
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, size.x, size.y);
}

void window::clear() {
	glClearColor(col_clear.r, col_clear.g, col_clear.b, col_clear.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void window::end_draw() {
	glfwSwapBuffers(winp);
	glfwPollEvents();
}

inline void window::set_current() {
	glfwMakeContextCurrent(winp);
}

bool window::CloseSignal() {
	return (glfwGetKey(winp, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(winp) == 0);
}

GLFWwindow* window::geth() {
	return winp;
}

vec2 ogl::window::cursor(bool normalized) {
	glm::vec<2, double, glm::defaultp> mousepos;
	glfwGetCursorPos(winp, &mousepos.x, &mousepos.y);

	vec2 out(mousepos.x, mousepos.y);

	if (normalized) {
		out.x = (out.x / size.x - 0.5) * 2;
		out.y = -(out.y / size.y - 0.5) * 2;
	}

	return out;
}

window::~window() {
	glfwDestroyWindow(winp);
}

