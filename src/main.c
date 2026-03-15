#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <m4xdev/types.h>
#include <m4xdev/logging.h>

#include "chunks/world.h"
#include "graphics/texture-atlas.h"
#include "state.h"
#include "camera.h"
#include "block-model.h"
#include "graphics/shader.h"

static void init(struct WindowState *window, struct AppState *app) {
	(void)window;

	LOG_TRACE("Initializing Voxel Game");

	shader_init_registry();
	blocks_init_registry();

	if (textureatlas_create(0, "resources/textures/blocks.png", 16, 16, 6, &app->tile_atlas) != 0) {
		LOG_FATAL("Failed to load atlas!");
	}

	if (world_create(CHUNK_RADIUS, &app->world) != 0) {
		LOG_FATAL("Failed to create world manager.");
	}

	world_preload(app->world, 0, 0);

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glfwSetInputMode(window->handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

static void destroy(struct WindowState *window, struct AppState *app) {
	(void)window;
	LOG_TRACE("Cleaning up Voxel Game");

	world_destroy(app->world);
	app->world = NULL;

	textureatlas_destroy(&app->tile_atlas);
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

	if (window->keyboard[GLFW_KEY_T] == BUTTON_DOWN) {
		LOG_DEBUG("Toggling Face culling");

		app->renderer.facecull = !app->renderer.facecull;
		if (app->renderer.facecull) {
			glEnable(GL_CULL_FACE);
		} else {
			glDisable(GL_CULL_FACE);
		}
	}

	if (window->keyboard[GLFW_KEY_Y] == BUTTON_DOWN) {
		LOG_DEBUG("Toggling antialiasing");

		app->renderer.antialias = !app->renderer.antialias;
		if (app->renderer.antialias) {
			glEnable(GL_MULTISAMPLE);
		} else {
			glDisable(GL_MULTISAMPLE);
		}
	}

	camera_update(&app->renderer.camera, window);

	world_update(app->world, app->renderer.camera.position[0], app->renderer.camera.position[2]);
}

static void render(struct WindowState *window, struct AppState *app) {
	(void)window;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Shader must be bound to update uniform
	shader_bind(Shaders[TERRAIN_SHADER]);
	textureatlas_bind(&app->tile_atlas);

	shader_uniform_mat4(Shaders[TERRAIN_SHADER], "view", app->renderer.camera.view);
	shader_uniform_mat4(Shaders[TERRAIN_SHADER], "projection", app->renderer.camera.projection);

	world_render(app->world, app->renderer.camera.position[0], app->renderer.camera.position[2]);
}

int main(void) {
	int width = 1280;
	int height = 720;
	float aspect = (float)width / (float)height;

	struct WindowConfig config = {
		.title = "Unnamed Voxel Game",
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
			.facecull = true,
			.antialias = true,
			.camera = camera_create(aspect)}};

	window_create(config, &state);
}
