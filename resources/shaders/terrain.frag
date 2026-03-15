#version 330 core

uniform sampler2DArray atlas;

flat in uint v_texture_layer;
in vec2 v_texture_uv;
in float v_ao;

out vec4 FragColor;

void main()
{
    FragColor = texture(atlas, vec3(v_texture_uv, v_texture_layer)) * v_ao;
}