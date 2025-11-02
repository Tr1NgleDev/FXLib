#version 430 core

layout(location = 0) in vec4 fsNormal;
layout(location = 1) in vec4 fsVertNormal;
layout(location = 2) in vec4 fsTangent;
layout(location = 3) in vec4 fsVertTangent;
layout(location = 4) in vec4 fsBiTangent;
layout(location = 5) in vec4 fsVertBiTangent;
layout(location = 6) in vec4 fsColor;
layout(location = 7) in vec3 fsUVW;

layout(location = 0) out vec4 color;

layout(location = 26) uniform float time;
layout(location = 27) uniform vec4 inColor;

void main()
{
	//color = vec4(fsUVW, 1.0);
	color = fsColor;
}