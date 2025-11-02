#version 330 core

layout(location = 0) in vec2 vert;

out vec2 uv;

void main()
{
	uv = vert * 0.5 + 0.5;
	gl_Position = vec4(vert, 0.0, 1.0);
}
