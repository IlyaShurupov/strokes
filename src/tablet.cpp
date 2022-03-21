
#include "tablet.h"

#include <Windows.h>

#define EASYTAB_IMPLEMENTATION
#include "easytab.h"

#include <stdio.h>

static float prev_pp = 0;

void tablet_init(int* win) {
	HWND w32window = (HWND)win;
	if (EasyTab_Load(w32window) != EASYTAB_OK) {
		printf("Tablet Is Not Initialized");
	}
}

void tablet_update(int* win) {
	HWND w32window = (HWND)win;
	MSG msg;
	PeekMessageW(&msg, NULL, 0, 0, PM_NOREMOVE);
	if (EasyTab_HandleEvent(msg.hwnd, msg.message, msg.lParam, msg.wParam) == EASYTAB_OK) {
		prev_pp = EasyTab->Pressure;
	}
}

float tablet_pressure() {
	return prev_pp;
}
