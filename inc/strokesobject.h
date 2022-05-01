
#pragma once

#include "object/object.h"
#include "strokes.h"

struct StrokesObject : Object {
	strokes_project project;

	static void copy(StrokesObject* self, const StrokesObject* in);
	static void destructor(StrokesObject* self);
	static void constructor(StrokesObject* self);
};

extern ObjectType StrokesObjectType;