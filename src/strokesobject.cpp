
#include "strokesobject.h"

using namespace tp;
using namespace obj;

void StrokesObject::constructor(StrokesObject* self) {
	new (&self->project) strokes_project();
}

void StrokesObject::copy(StrokesObject* self, const StrokesObject* in) {
	self->project = in->project;
}

void StrokesObject::destructor(StrokesObject* self) {
	self->project.~strokes_project();
}

static alni save_size(StrokesObject* self) {
	return self->project.save_size();
}

static void save(StrokesObject* self, File& file_self) {
	self->project.save(file_self);
}

static void load(File& file_self, StrokesObject* self) {
	new (&self->project) strokes_project();
	self->project.load(file_self);
}

struct ObjectType obj::StrokesObject::TypeData = {
	.base = NULL,
	.constructor = (object_constructor) StrokesObject::constructor,
	.destructor = (object_destructor) StrokesObject::destructor,
	.copy = (object_copy) StrokesObject::copy,
	.size = sizeof(StrokesObject),
	.name = "strokes",
	.save_size = (object_save_size) save_size,
	.save = (object_save) save,
	.load = (object_load) load,
};
