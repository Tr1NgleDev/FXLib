#version 430 core

layout(lines_adjacency) in;
layout(triangle_strip, max_vertices = 4) out;

in int instanceID[];
out flat int fsInstanceID;

// model view matrix
//uniform float MV[25];

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
uniform float[25] view;

// projection matrix is handled here after the coordinates are converted to 3D
uniform mat4 P;

// tetrahedron edge vertex indices
const ivec2 edges[6] = ivec2[6](ivec2(0, 1), ivec2(0, 2), ivec2(0, 3), ivec2(1, 2), ivec2(1, 3), ivec2(2, 3));

void main()
{
	int instID = instanceID[0];

	if (data[instID].t >= 1.0) return;

	fsInstanceID = instID;
	
	// calculate intersection between simplex and hyperplane (3D view)

	// number of intersection points
	int k = 0;
	for (int i = 0; i < 6; ++i)
	{
		if (k == 4)
		{
			break;
		}

		ivec2 e = edges[i];
		vec4 p0 = gl_in[e[0]].gl_Position;
		vec4 p1 = gl_in[e[1]].gl_Position;

		// vertices are on the same side of the hyperplane so the edge doesn't intersect
		if ((p0[3] < 0.00000001 && p1[3] < 0.00000001) || (p0[3] > -0.00000001 && p1[3] > -0.00000001))
		{
			continue;
		}

		// intersection
		float a = 0.0;
		if (abs(p1[3] - p0[3]) > 0.00000001)
		{
			a = (-p0[3]) / (p1[3] - p0[3]);
		}
		vec4 pt = mix(p0, p1, a);

		gl_Position = P * vec4(pt.xyz, 1.0);

		EmitVertex();

		++k;
	}

	EndPrimitive();
}