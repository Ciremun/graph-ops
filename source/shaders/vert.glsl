#version 300 es

precision highp float;

layout(location = 0) in vec3 vert_pos;

uniform mat4 MVP;
uniform float u_time;

void main()
{
    gl_Position = MVP * vec4(vert_pos, 1);
}
