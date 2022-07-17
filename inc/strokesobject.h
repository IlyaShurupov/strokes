
#pragma once

#include "object/object.h"
#include "strokes.h"

namespace obj {

	struct StrokesObject : Object {
		strokes_project project;
		static struct ObjectType TypeData;

		static void copy(StrokesObject* self, const StrokesObject* in);
		static void destructor(StrokesObject* self);
		static void constructor(StrokesObject* self);
	};


};