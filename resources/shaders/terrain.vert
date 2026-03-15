#version 330 core

const float ao_curve[] = float[]( 0.2, 0.4, 0.8, 1.0 );

layout (location = 0) in vec3 position;
layout (location = 1) in uint data;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

flat out uint v_texture_layer;
out vec2 v_texture_uv;
out float v_ao;


void main() {
	v_ao = ao_curve[data & 3u];

	int vid = gl_VertexID % 4;

	v_texture_layer = data >> 2;
    v_texture_uv = vec2(
		(vid == 1 || vid == 2) ? 1.0 : 0.0,
		(vid >= 2) ? 1.0 : 0.0
	);

	gl_Position = projection * view * model * vec4(position, 1.0);
}