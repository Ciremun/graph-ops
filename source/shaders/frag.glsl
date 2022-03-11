#version 330 core

out vec4 color;

uniform float u_time;
uniform vec4 u_color;

void main()
{
    color = u_color;
}
