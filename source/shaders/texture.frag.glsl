#version 300 es

precision highp float;

in vec2 uv;
out vec4 color;

uniform float u_time;
uniform vec4 u_color;
uniform sampler2D s;

void main()
{
    color = texture(s, uv);
}
