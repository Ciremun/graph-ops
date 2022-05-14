#version 300 es

precision highp float;

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;

out vec2 uv;

uniform mat4 u_mvp;
uniform float u_time;

void main()
{
    gl_Position = u_mvp * vec4(a_pos, 1);
    uv = a_uv;
}
