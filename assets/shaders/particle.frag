#version 430 core

in flat int fsInstanceID;

// Ouput data
layout (location=0) out vec4 color;

struct InstanceData
{
	float[25] model;
	float[4] scale;
	float[4] color;
	float t;
};
layout(std430, binding = 0) readonly buffer instanceData
{
    InstanceData data[];
};

void main()
{
	color = 
	vec4(
			data[fsInstanceID].color[0],
			data[fsInstanceID].color[1],
			data[fsInstanceID].color[2],
			data[fsInstanceID].color[3]
		);
}