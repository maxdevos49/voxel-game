#ifndef GFX_WINDOW_H
#define GFX_WINDOW_H

#include <cglm/cglm.h>
#include "graphics.h"

enum ButtonState {
	BUTTON_IDLE,
	BUTTON_DOWN,
	BUTTON_HELD,
	BUTTON_UP
};

struct WindowState {
	GLFWwindow *handle;

	struct {
		int width;
		int height;
	} size;

	struct {
		double now, last;
		double accumulator; // accumulator += frameDelta
		float frameDelta;	// limit(now - last,  MAX_FRAME_TIME)
		float tickDelta;	// limit(accumulator / FIXED_TIMESTEP), 1)
		uint64_t tick;
	} time;

	enum ButtonState keyboard[GLFW_KEY_LAST + 1];

	struct {
		enum ButtonState button[GLFW_MOUSE_BUTTON_LAST + 1];
		vec2 position;
		vec2 delta;
		// struct {
		// 	double xOffset;
		// 	double yOffset;
		// } scroll;
	} mouse;

	bool paused;
};

struct AppState;
typedef void (*WHandler)(struct WindowState *window, struct AppState *app);

struct WindowConfig {
	const char *title;
	struct {
		int width;
		int height;
	} size;

	WHandler init, destroy, tick, update, render;
};

void window_create(struct WindowConfig config, struct AppState *app);

#endif
