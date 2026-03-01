#include "cglm/util.h"
#define GLAD_GL_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include <m4xdev/logging.h>

#include "window.h"

static const float FIXED_TIMESTEP = 1.0F / 20.0F; //TODO  Add to window state
static const float MAX_FRAME_TIME = 0.25F;

static void glfw_error_callback(int error_code, const char *description) {
	fprintf(stderr, "GLFW Error(%d): %s\n", error_code, description);
}

static void glfw_framebuffer_size_callback(GLFWwindow *handle, int width, int height) {
	glViewport(0, 0, width, height);

	struct WindowState *window = glfwGetWindowUserPointer(handle);
	assert(window != NULL && "GLFW user pointer is NULL");

	window->size.width = width;
	window->size.height = height;
}

static bool keyboardButtonIsDown[GLFW_KEY_LAST + 1] = {0};

static void glfw_key_callback(GLFWwindow *handle, int key, int scancode, int action, int mod) {
	(void)handle;
	(void)scancode;
	(void)mod;

	if(key < 0) return;

	keyboardButtonIsDown[key] = (action == GLFW_PRESS || action == GLFW_REPEAT);
}

static bool mouseButtonIsDown[GLFW_MOUSE_BUTTON_LAST + 1] = {0};

static void glfw_mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
	(void)window;
	(void)mods;

	mouseButtonIsDown[button] = action == GLFW_PRESS;
}

static void window_performance(struct WindowState *window) {
	static double last_log_time = 0.0;
	static double last_log_tick = 0.0;
	static int frames = 0;

	frames++;

	if (window->time.now - last_log_time >= 1.0) {
		double seconds = window->time.now - last_log_time;

		double fps = ((double)frames + window->time.frameDelta) / seconds;
		double frame_ns = (seconds / ((double)frames + window->time.frameDelta)) * 1000.0 * 1000.0; // ns per frame
		double tps = (((double)window->time.tick - last_log_tick) + window->time.tickDelta) / seconds;
		double tick_us = window->time.tickDelta * 1000.0; // µs

		LOG_DEBUG("FPS: %.2f | Frame Delta: %.2f ns | TPS: %.2f | Tick Delta: %.2f µs | Tick: %llu",
			fps, frame_ns, tps, tick_us, window->time.tick);

		if (tps < 19.0) {
			LOG_WARN("TPS dropped to %.2f (target >= 20)", tps);
		}
		if (fps < 60.0) {
			LOG_WARN("FPS dropped to %0.2f (target ≥60)", fps);
		}

		frames = 0;
		last_log_time = window->time.now;
		last_log_tick = (double)window->time.tick + window->time.tickDelta;
	}
}

void window_create(struct WindowConfig config, struct AppState *app) {
	glfwSetErrorCallback(glfw_error_callback);

	if (GLFW_FALSE == glfwInit()) {
		fprintf(stderr, "Error initializing GLFW\n");
		exit(EXIT_FAILURE);
	}

#ifdef DEBUG
	int major, minor, revision = 0;
	glfwGetVersion(&major, &minor, &revision);
	printf("Initialized GLFW v%i.%i.%i\n", major, minor, revision);
#endif

	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	struct WindowState window = {0};

	window.handle = glfwCreateWindow(config.size.width, config.size.height, config.title, NULL, NULL);
	if (window.handle == NULL) {
		glfwTerminate();
		fprintf(stderr, "Error creating window\n");
		exit(EXIT_FAILURE);
	}

	glfwSetWindowUserPointer(window.handle, &window);
	glfwSetFramebufferSizeCallback(window.handle, glfw_framebuffer_size_callback);
	glfwSetKeyCallback(window.handle, glfw_key_callback);
	glfwSetMouseButtonCallback(window.handle, glfw_mouse_button_callback);

	glfwMakeContextCurrent(window.handle);

	int version = gladLoadGL(glfwGetProcAddress);
	if (version == 0) {
		fprintf(stderr, "Error initializing GLAD\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

#ifdef DEBUG
	printf("Initialized GLAD with OpenGL v%d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
#endif

	glfwSwapInterval(1);

	config.init(&window, app);

	window.time.now = glfwGetTime();
	window.time.last = window.time.now;
	window.time.accumulator = 0.0;
	window.time.frameDelta = 0.0F;
	window.time.tickDelta = 0.0F;
	window.time.tick = 0;

	while (!glfwWindowShouldClose(window.handle)) {
		window.time.now = glfwGetTime();
		window.time.frameDelta = glm_clamp(window.time.now - window.time.last, 0, MAX_FRAME_TIME);

		window.time.accumulator += window.time.frameDelta;
		window.time.last = window.time.now;

		glfwPollEvents();

		//
		// Process keyboard state.
		for (int key = 0; key <= GLFW_KEY_LAST; key++) {
			bool down = keyboardButtonIsDown[key];

			switch (window.keyboard[key]) {
			case BUTTON_IDLE:
			case BUTTON_UP:
				window.keyboard[key] = down ? BUTTON_DOWN : BUTTON_IDLE;
				break;

			case BUTTON_DOWN:
			case BUTTON_HELD:
				window.keyboard[key] = down ? BUTTON_HELD : BUTTON_UP;
				break;
			}
		}

		//
		// Process mouse button state.
		for (int button = 0; button <= GLFW_MOUSE_BUTTON_LAST; button++) {
			bool down = mouseButtonIsDown[button];

			switch (window.mouse.button[button]) {
			case BUTTON_IDLE:
			case BUTTON_UP:
				window.mouse.button[button] = down ? BUTTON_DOWN : BUTTON_IDLE;
				break;

			case BUTTON_DOWN:
			case BUTTON_HELD:
				window.mouse.button[button] = down ? BUTTON_HELD : BUTTON_UP;
				break;
			}
		}

		//
		// Process Mouse input
		static bool initial = true;
		double xPos, yPos;
		glfwGetCursorPos(window.handle, &xPos, &yPos);
		if (initial) {
			window.mouse.position[0] = xPos;
			window.mouse.position[1] = yPos;
			initial = false;
		}

		window.mouse.delta[0] = glm_clamp(xPos - window.mouse.position[0], -100.0f, 100.0f);
		window.mouse.delta[1] = glm_clamp(yPos - window.mouse.position[1], -100.0f, 100.0f);
		window.mouse.position[0] = xPos;
		window.mouse.position[1] = yPos;

		//
		// Tick Processing
		while (window.time.accumulator >= FIXED_TIMESTEP) {
			window.time.accumulator -= FIXED_TIMESTEP;
			window.time.tick++;

			//
			// Tick Step - Handle world simulation, entity processing, ... etc
			config.tick(&window, app);
		}

		window.time.tickDelta = glm_clamp(window.time.accumulator / FIXED_TIMESTEP,0, 1.0);

		//
		// Update Step - Handle generating chunks, rebuilding meshes, ... etc
		config.update(&window, app);

		// Render Step - Send the fruits of our labor to the GPU
		config.render(&window, app);

		// Let their be results!
		glfwSwapBuffers(window.handle);

		window_performance(&window);
	}

	config.destroy(&window, app);

	glfwDestroyWindow(window.handle);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}
