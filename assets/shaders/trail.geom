#version 430 core

layout(lines_adjacency) in;
layout(triangle_strip, max_vertices = 4) out;

layout(location = 0) in vec4 gsNormal[];
layout(location = 1) in vec4 gsVertNormal[];
layout(location = 2) in vec4 gsTangent[];
layout(location = 3) in vec4 gsVertTangent[];
layout(location = 4) in vec4 gsBiTangent[];
layout(location = 5) in vec4 gsVertBiTangent[];
layout(location = 6) in vec4 gsColor[];
layout(location = 7) in vec3 gsUVW[];

layout(location = 0) out vec4 fsNormal;
layout(location = 1) out vec4 fsVertNormal;
layout(location = 2) out vec4 fsTangent;
layout(location = 3) out vec4 fsVertTangent;
layout(location = 4) out vec4 fsBiTangent;
layout(location = 5) out vec4 fsVertBiTangent;
layout(location = 6) out vec4 fsColor;
layout(location = 7) out vec3 fsUVW;

// projection matrix is handled here after the coordinates are converted to 3D
layout(location = 25) uniform mat4 P;

// tetrahedron edge vertex indices
const ivec2 edges[6] = ivec2[6](ivec2(0, 1), ivec2(0, 2), ivec2(0, 3), ivec2(1, 2), ivec2(1, 3), ivec2(2, 3));

void main()
{
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
		if ((p0.w < 0.0000001 && p1.w < 0.0000001) || (p0.w > -0.0000001 && p1.w > -0.0000001))
		{
			continue;
		}

		// intersection
		float a = 0;
		if (abs(p1.w - p0.w) > 0.0000001)
		{
			a = (-p0.w) / (p1.w - p0.w);
		}

		vec4 pt = mix(p0, p1, a);
		gl_Position = P * vec4(pt.xyz, 1.0);

		fsNormal = mix(gsNormal[e[0]], gsNormal[e[1]], a);
		fsVertNormal = mix(gsVertNormal[e[0]], gsVertNormal[e[1]], a);
		fsTangent = mix(gsTangent[e[0]], gsTangent[e[1]], a);
		fsVertTangent = mix(gsVertTangent[e[0]], gsVertTangent[e[1]], a);
		fsBiTangent = mix(gsTangent[e[0]], gsTangent[e[1]], a);
		fsVertBiTangent = mix(gsVertTangent[e[0]], gsVertTangent[e[1]], a);
		fsColor = mix(gsColor[e[0]], gsColor[e[1]], a);
		fsUVW = mix(gsUVW[e[0]], gsUVW[e[1]], a);

		EmitVertex();

		++k;
	}
	
	EndPrimitive();
}