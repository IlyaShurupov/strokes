

#include "gl.h"
#include "window.h"
#include "fbuffer.h"
#include "glutils.h"

#include "strokes.h"
#include "controller.h"

#include "GUI.h"

class StrokeApp {

	ogl::opengl ogl;
	ogl::window window;
	ogl::fbuffer fbo;

	GuiState gui;
	camera cam;
	drawlayer layer;
	inputsmpler sampler;

	bool quit = false;

public:

	StrokeApp(vec2 size) : ogl(), window(size), fbo(size, vec4(0)), gui(&window) {
	}
	
	void proc_inputs() {

		camera_controller(window.geth(), &cam);
		sampler.sample(window.cursor(true), window.pen_pressure(), &cam);

		if (!sampler.active_state() && sampler.has_input()) {
			layer.add_stroke(sampler.get_stroke());
			sampler.clear();
		}
	}
	
	void send_output() {

		window.begin_draw(); {
			window.clear();

			layer.draw(cam.projmat(), cam.viewmat());
			sampler.draw(cam.projmat(), cam.viewmat());

			//gui.Icon(vec4(10, 10, 100, 120), "A:/src/ogl/rsc/icons/Star.png");

			if (gui.button(vec4(700, window.size.y - 80, 70, 70), NULL, "A:/src/ogl/rsc/icons/Backward.png")) {
				layer.undo();
			}
			if (gui.button(vec4(780, window.size.y - 80, 70, 70), NULL, "A:/src/ogl/rsc/icons/Forward.png")) {
				layer.redo();
			}
			if (gui.button(vec4(860, window.size.y - 80, 70, 70), NULL, "A:/src/ogl/rsc/icons/quit.png")) {
				quit = 1;
			}

		} window.end_draw();
	}

	bool IsRunnign() {
		return !quit && window.CloseSignal();
	}
};


int main() {
	
	StrokeApp app(vec2(1920, 1080));
	frame_counter fc;

	do {

		fc.log();

		app.proc_inputs();
		app.send_output();

	} while (app.IsRunnign());
}