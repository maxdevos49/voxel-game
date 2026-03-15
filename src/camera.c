#include <assert.h>
#include <math.h>
#include "camera.h"

struct Camera camera_create(float initial_aspect) {
	struct Camera self = {0};

	self.position[0] = 0.0f;
	self.position[1] = 70.0f;
	self.position[2] = 0.0f;

	self.up[0] = 0.0f;
	self.up[1] = 1.0f;
	self.up[2] = 0.0f;

	self.worldUp[0] = 0.0f;
	self.worldUp[1] = 1.0f;
	self.worldUp[2] = 0.0f;

	self.yaw = -glm_rad(90);
	self.pitch = 0.0f;

	self.sensitivity = 0.005f;
	self.fov = 45.0f;
	self.znear = 0.1f;
	self.zfar = 10000.0f;

	self.acceleration = 100;
	self.damping = 0.2f;
	self.maxSpeed = 100;

	self.aspect = initial_aspect;
	glm_perspective(self.fov, initial_aspect, self.znear, self.zfar, self.projection);

	self.front[0] = cosf(self.yaw) * cosf(self.pitch);
	self.front[1] = sinf(self.pitch);
	self.front[2] = sinf(self.yaw) * cosf(self.pitch);
	glm_vec3_normalize(self.front);

	glm_vec3_crossn(self.front, self.worldUp, self.right);
	glm_vec3_crossn(self.right, self.front, self.up);

	return self;
}

void camera_update(struct Camera *self, struct WindowState *window) {
	assert(self != NULL);
	assert(window != NULL);

	if (window->paused) {
		return;
	}

	vec3 deltaV = {0};
	float acceleration = self->acceleration * window->time.frameDelta;
	if (window->keyboard[GLFW_KEY_W]) {
		glm_vec3_scale(self->front, acceleration, deltaV);
		glm_vec3_add(self->velocity, deltaV, self->velocity);
	}

	if (window->keyboard[GLFW_KEY_A]) {
		glm_vec3_scale(self->right, acceleration, deltaV);
		glm_vec3_sub(self->velocity, deltaV, self->velocity);
	}

	if (window->keyboard[GLFW_KEY_S]) {
		glm_vec3_scale(self->front, acceleration, deltaV);
		glm_vec3_sub(self->velocity, deltaV, self->velocity);
	}

	if (window->keyboard[GLFW_KEY_D]) {
		glm_vec3_scale(self->right, acceleration, deltaV);
		glm_vec3_add(self->velocity, deltaV, self->velocity);
	}

	if (window->keyboard[GLFW_KEY_SPACE]) {
		glm_vec3_scale(self->up, acceleration, deltaV);
		glm_vec3_add(self->position, deltaV, self->position);
	}

	if (window->keyboard[GLFW_KEY_LEFT_SHIFT]) {
		glm_vec3_scale(self->up, acceleration, deltaV);
		glm_vec3_sub(self->position, deltaV, self->position);
	}

	float dampingFactor = powf(self->damping, window->time.frameDelta);
	glm_vec3_scale(self->velocity, dampingFactor, self->velocity);

	float maxSpeed = self->maxSpeed;
	float currentSpeed = glm_vec3_norm(self->velocity);
	if (currentSpeed > maxSpeed) {
		glm_vec3_normalize(self->velocity);
		glm_vec3_scale(self->velocity, maxSpeed, self->velocity);
	}

	vec3 positionDelta;
	glm_vec3_scale(self->velocity, window->time.frameDelta, positionDelta);
	glm_vec3_add(self->position, positionDelta, self->position);

	self->yaw += window->mouse.delta[0] * self->sensitivity;
	self->pitch -= window->mouse.delta[1] * self->sensitivity;

	self->pitch = glm_clamp(self->pitch, -M_PI_2, M_PI_2);

	self->front[0] = cosf(self->yaw) * cosf(self->pitch);
	self->front[1] = sinf(self->pitch);
	self->front[2] = sinf(self->yaw) * cosf(self->pitch);
	glm_vec3_normalize(self->front);

	glm_vec3_crossn(self->front, self->worldUp, self->right);
	glm_vec3_crossn(self->right, self->front, self->up);

	vec3 focusPoint;
	glm_vec3_add(self->position, self->front, focusPoint);
	glm_lookat(self->position, focusPoint, self->up, self->view);
}
