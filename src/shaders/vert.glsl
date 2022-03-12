#version 300 es

precision highp float;

layout(location = 0) in vec3 vert_pos;

uniform mat4 MVP;
uniform float u_time;

void main()
{
    vec3 vp = vert_pos;
    vp.y += sin(u_time);
    gl_Position = MVP * vec4(vp, 1);
}
