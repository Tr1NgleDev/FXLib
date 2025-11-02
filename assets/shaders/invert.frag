#version 330 core

layout(location = 0) out vec4 color;

in vec2 uv;

uniform sampler2D source;
uniform vec4 sourceSize; // w, h, 1/w, 1/h

void main()
{
	vec4 s = texture(source, uv);
	color.rgb = 1.0 - s.rgb;
	color.a = s.a;
}
