#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <m4xdev/types.h>
#include <m4xdev/logging.h>

#include "state.h"
#include "graphics/shader.h"
#include "graphics/texture.h"
#include "camera.h"
#include "block-model.h"
#include "chunk.h"

static void init(struct WindowState *window, struct AppState *app) {
	(void)window;

	LOG_TRACE("Initializing Voxel Game");

	shader_init_registry();
	texture_init_registry();
	blocks_init_registry();

	i32 chunk_radius = CHUNK_DIAMETER / 2;

	for (i32 i = 0; i < CHUNK_COUNT; i++) {
		i32 x = (i % CHUNK_DIAMETER) - chunk_radius;
		i32 z = (i / CHUNK_DIAMETER) - chunk_radius;

		i32 err = 0;
		if ((err = generate_chunk(x, z, &app->chunks[i])) != 0) {
			LOG_FATAL("Failed to generate chunk!: %d\n", err);
		}
	}

	for (i32 i = 0; i < CHUNK_COUNT; i++) {
		struct Chunk *chunk = app->chunks[i];

		struct Chunk *north_chunk = i - CHUNK_DIAMETER >= 0 ? app->chunks[i - CHUNK_DIAMETER] : NULL;
		struct Chunk *east_chunk = i + 1 < CHUNK_COUNT ? app->chunks[i + 1] : NULL;
		struct Chunk *south_chunk = i + CHUNK_DIAMETER < CHUNK_COUNT ? app->chunks[i + CHUNK_DIAMETER] : NULL;
		struct Chunk *west_chunk = i - 1 >= 0 ? app->chunks[i - 1] : NULL;

		// LOG_DEBUG("X: %d Z: %d", x, z);

		i32 err = 0;
		if ((err = mesh_chunk(chunk, north_chunk, east_chunk, south_chunk, west_chunk)) != 0) {
			LOG_FATAL("Failed to mesh chunk!: %d\n", err);
		}
	}

	glfwSetInputMode(window->handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

static void destroy(struct WindowState *window, struct AppState *app) {
	(void)window;
	LOG_TRACE("Cleaning up Voxel Game");

	for (int i = 0; i < CHUNK_COUNT; i++) {
		struct Chunk *chunk = app->chunks[i];
		if (chunk != NULL) {
			destroy_chunk(chunk);
			app->chunks[i] = NULL;
		}
	}

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

	for (int i = 0; i < CHUNK_COUNT; i++) {
		struct Chunk *chunk = app->chunks[i];
		if (chunk != NULL) {
			mat4 model;
			glm_mat4_identity(model);

			vec3 chunk_position = {chunk->chunkX * CHUNK_WIDTH, 0, chunk->chunkZ * CHUNK_WIDTH};
			glm_translate(model, chunk_position);

			shader_uniform_mat4(Shaders[TERRAIN_SHADER], "model", model);

			render_chunk(chunk);
		}
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
