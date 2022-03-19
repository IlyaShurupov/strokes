

#include "gl.h"
#include "window.h"
#include "fbuffer.h"
#include "glutils.h"

#include "strokes.h"
#include "controller.h"

class Drawer {

	ogl::opengl ogl;
	ogl::window window;
	ogl::fbuffer fbo;

	camera cam;
	drawlayer layer;
	inputsmpler sampler;

public:

	Drawer() : ogl(), window(), fbo(vec2(1024, 768), vec4(0)) {
	}
	
	void proc_inputs() {

		camera_controller(window.geth(), &cam);
		sampler.sample(window.cursor(true), glfwGetMouseButton(window.geth(), GLFW_MOUSE_BUTTON_1) == GLFW_PRESS, &cam);

		if (!sampler.active_state() && sampler.has_input()) {
			layer.add_stroke(sampler.get_stroke());
			sampler.clear();
		}
	}
	
	void send_output() {

		fbo.begin_draw(); {
			fbo.clear();

			layer.draw(cam.projmat(), cam.viewmat());
			sampler.draw(cam.projmat(), cam.viewmat());

		} fbo.end_draw();

		window.begin_draw(); {
			window.clear();

			draw_texture(0, fbo.texId(), vec4(200, 200, 500, 3.f / 4 * 500), vec4(200, 200, 500, 3.f / 4 * 500));

		} window.end_draw();
	}

	bool IsRunnign() {
		return window.CloseSignal();

	}

};

int main() {

	Drawer app;
	frame_counter fc;

	do {

		fc.log();

		app.proc_inputs();
		app.send_output();

	} while (app.IsRunnign());
}