#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <m4xdev/types.h>
#include <m4xdev/logging.h>

#include "state.h"
#include "camera.h"
#include "block-model.h"
#include "graphics/shader.h"
#include "graphics/texture.h"

static void init(struct WindowState *window, struct AppState *app) {
	(void)window;

	LOG_TRACE("Initializing Voxel Game");

	shader_init_registry();
	texture_init_registry();
	blocks_init_registry();

	if (chunkmanager_create(CHUNK_RADIUS, &app->chunk_manager) != 0) {
		LOG_FATAL("Failed to create chunk manager.");
	}

	glfwSetInputMode(window->handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

static void destroy(struct WindowState *window, struct AppState *app) {
	(void)window;
	(void)app;
	LOG_TRACE("Cleaning up Voxel Game");

	// TODO destroy manager

	// TODO free gl texture buffers
	// texture_destroy_registry();
	shader_destroy_registry();
}

static void tick(struct WindowState *window, struct AppState *app) {
	(void)window;
	(void)app;
}

static void update(struct WindowState *window, struct AppState *app) {
	if (window->keyboard[GLFW_KEY_Q] == BUTTON_DOWN) {
		glfwSetWindowShouldClose(window->handle, GLFW_TRUE);
	}

	if (window->keyboard[GLFW_KEY_Y] == BUTTON_DOWN) {
		LOG_DEBUG("Toggling Wireframe");

		app->renderer.wireframe = !app->renderer.wireframe;
		if (!app->renderer.wireframe) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		} else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
	}

	if (window->keyboard[GLFW_KEY_V] == BUTTON_DOWN) {
		LOG_DEBUG("Toggling VSync");

		app->renderer.vsync = !app->renderer.vsync;
		if (app->renderer.vsync) {
			glfwSwapInterval(1);
		} else {
			glfwSwapInterval(0);
		}
	}

	if (window->keyboard[GLFW_KEY_ESCAPE] == BUTTON_DOWN) {
		LOG_DEBUG("Toggling Pause");

		window->paused = !window->paused;
		if (window->paused) {
			glfwSetInputMode(window->handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		} else {
			glfwSetInputMode(window->handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
	}

	static bool faceCulling = false;
	if (window->keyboard[GLFW_KEY_T] == BUTTON_DOWN) {
		LOG_DEBUG("Toggling Face culling");

		faceCulling = !faceCulling;
		if (faceCulling) {
			glEnable(GL_CULL_FACE);
		} else {
			glDisable(GL_CULL_FACE);
		}
	}

	camera_update(&app->renderer.camera, window);

	if (chunkmanager_update(app->chunk_manager, app->renderer.camera.position[0], app->renderer.camera.position[2]) != 0) {
		LOG_FATAL("Failed to update chunks!");
	}
}

static void render(struct WindowState *window, struct AppState *app) {
	(void)window;
	// TODO move out?
	glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Shader must be bound to update uniform
	shader_bind(Shaders[TERRAIN_SHADER]);
	texture_bind(Textures[TERRAIN_TEXTURE]);

	shader_uniform_mat4(Shaders[TERRAIN_SHADER], "view", app->renderer.camera.view);
	shader_uniform_mat4(Shaders[TERRAIN_SHADER], "projection", app->renderer.camera.projection);

	if (chunkmanager_render(app->chunk_manager, app->renderer.camera.position[0], app->renderer.camera.position[2]) != 0) {
		LOG_FATAL("Failed to render chunks!");
	}
}

int main(void) {
	int width = 1280;
	int height = 720;
	float aspect = (float)width / (float)height;

	struct WindowConfig config = {
		.title = "Voxel Game",
		.size = {
			.width = width,
			.height = height},
		.init = init,
		.destroy = destroy,
		.tick = tick,
		.update = update,
		.render = render};

	struct AppState state = {
		.renderer = {
			.wireframe = false,
			.vsync = true,
			.camera = camera_create(aspect)}};

	window_create(config, &state);
}
