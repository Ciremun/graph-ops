#version 300 es

precision highp float;

layout(location = 0) in vec3 vert_pos;

uniform mat4 u_mvp;
uniform float u_time;

void main()
{
    gl_Position = u_mvp * vec4(vert_pos, 1);
}
