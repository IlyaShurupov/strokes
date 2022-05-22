
#include "app.h"

using namespace tp;
using namespace obj;

int main() {

	objects_init();
	primitives_define_types();
	NDO->define(&StrokesObjectType);

	{
		StrokeApp app;
		app.Run();
	}

	objects_finalize();
	return 0;
}

int WinMain() {
	return main();
}