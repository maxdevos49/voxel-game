#ifndef CAMERA_H
#define CAMERA_H 1

#include <cglm/cglm.h>
#include "graphics/window.h"

struct Camera {
	vec3 position, velocity;
	vec3 worldUp, up, front, right;
	float pitch, yaw;
	float sensitivity, fov, znear, zfar;
	float maxSpeed, acceleration, damping;
	float aspect;
	mat4 view;
	mat4 projection;
};


struct Camera camera_create(float aspect);

void camera_update(struct Camera *self, struct WindowState* window);
// void camera_set_speed(struct Camera* self, float speed);
// void camera_set_acceleration(struct Camera* self, float accel);
// void camera_set_damping(struct Camera* self, float damping);
// void camera_set_fov(struct Camera* self, float fov);
// void camera_set_sensitivity(struct Camera* self, float sensitivity); // For mouse

// void camera_get_view(struct Camera *self, mat4 view);
// void camera_get_projection(struct Camera *self, mat4 projection);

#endif
