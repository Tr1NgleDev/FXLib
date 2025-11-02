#version 430 core

layout(location = 0) in vec4 vert;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec4 tangent;
layout(location = 3) in vec4 biTangent;
layout(location = 4) in vec4 color;
layout(location = 5) in vec3 uvw;

layout(location = 0) out vec4 gsNormal;
layout(location = 1) out vec4 gsVertNormal;
layout(location = 2) out vec4 gsTangent;
layout(location = 3) out vec4 gsVertTangent;
layout(location = 4) out vec4 gsBiTangent;
layout(location = 5) out vec4 gsVertBiTangent;
layout(location = 6) out vec4 gsColor;
layout(location = 7) out vec3 gsUVW;

layout(location = 0) uniform float MV[25];

vec4 Mat5_multiply(in float m[25], in vec4 v, in float finalComp)
{
	return vec4(
        m[0*5+0] * v[0] + m[1*5+0] * v[1] + m[2*5+0] * v[2] + m[3*5+0] * v[3] + m[4*5+0] * finalComp,
        m[0*5+1] * v[0] + m[1*5+1] * v[1] + m[2*5+1] * v[2] + m[3*5+1] * v[3] + m[4*5+1] * finalComp,
        m[0*5+2] * v[0] + m[1*5+2] * v[1] + m[2*5+2] * v[2] + m[3*5+2] * v[3] + m[4*5+2] * finalComp,
        m[0*5+3] * v[0] + m[1*5+3] * v[1] + m[2*5+3] * v[2] + m[3*5+3] * v[3] + m[4*5+3] * finalComp
    );
}

void main()
{
	// multiply the vertex by MV
	vec4 result = Mat5_multiply(MV, vert, 1.0);

	mat4 MVM4 = transpose(inverse(mat4(
		vec4(MV[0 * 5 + 0], MV[0 * 5 + 1], MV[0 * 5 + 2], MV[0 * 5 + 3]),
		vec4(MV[1 * 5 + 0], MV[1 * 5 + 1], MV[1 * 5 + 2], MV[1 * 5 + 3]),
		vec4(MV[2 * 5 + 0], MV[2 * 5 + 1], MV[2 * 5 + 2], MV[2 * 5 + 3]),
		vec4(MV[3 * 5 + 0], MV[3 * 5 + 1], MV[3 * 5 + 2], MV[3 * 5 + 3])
	)));

    vec4 resultN = normalize(MVM4 * normal);
    vec4 resultT = normalize(MVM4 * tangent);
    vec4 resultB = normalize(MVM4 * biTangent);

	gsNormal = resultN;
	gsVertNormal = normal;
	gsTangent = resultT;
	gsVertTangent = tangent;
	gsBiTangent = resultB;
	gsVertBiTangent = biTangent;
	gsColor = color;
	gsUVW = uvw;

	gl_Position = result;
}