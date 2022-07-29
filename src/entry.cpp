
#include "app.h"

using namespace tp;
using namespace obj;

int main() {

	alloc_init();
	{
	
		objects_init();
		primitives_define_types();
		NDO->define(&StrokesObject::TypeData);

		{
			StrokeApp app;
			app.Run();
		}

		objects_finalize();
	}
	
	alloc_uninit();
	return 0;
}

int WinMain() {
	return main();
}