
#include "app.h"

int main() {

	StrokeApp app(vec2f(600, 600));

	fpscount fc;

	do {
		fc.update(app.fps);
		app.mainloop();
	} while (app.IsRunnign());

	return 0;
}

int WinMain() {
	return main();
}