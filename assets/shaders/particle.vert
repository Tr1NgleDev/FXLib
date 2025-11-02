#version 430 core

layout(location = 0) in vec4 vert;

out int instanceID;

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
uniform bool billboard;

vec4 cross(vec4 u, vec4 v, vec4 w)
{
	//  intermediate values
	float a = (v.x * w.y) - (v.y * w.x);
	float b = (v.x * w.z) - (v.z * w.x);
	float c = (v.x * w.w) - (v.w * w.x);
	float d = (v.y * w.z) - (v.z * w.y);
	float e = (v.y * w.w) - (v.w * w.y);
	float f = (v.z * w.w) - (v.w * w.z);

	// result vector
	vec4 res;

	res.x = (u.y * f) - (u.z * e) + (u.w * d);
	res.y = -(u.x * f) + (u.z * c) - (u.w * b);
	res.z = (u.x * e) - (u.y * c) + (u.w * a);
	res.w = -(u.x * d) + (u.y * b) - (u.z * a);

	return res;
}

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

	vec4 v = (vert - vec4(0.5)) * vec4(data[gl_InstanceID].scale[0], data[gl_InstanceID].scale[1], data[gl_InstanceID].scale[2], data[gl_InstanceID].scale[3]);

	float[25] m = data[gl_InstanceID].model;
	if (billboard)
	{
		vec4 up = vec4(0, 1, 0, 0);
		vec4 f = normalize(cross(vec4(view[0 * 5 + 0],view[1 * 5 + 0],view[2 * 5 + 0],view[3 * 5 + 0]), vec4(0, 1, 0, 0), vec4(view[0 * 5 + 3],view[1 * 5 + 3],view[2 * 5 + 3],view[3 * 5 + 3])));
		vec4 l = normalize(cross(f, vec4(0, 1, 0, 0), vec4(view[0 * 5 + 3],view[1 * 5 + 3],view[2 * 5 + 3],view[3 * 5 + 3])));
		for (int i = 0; i < 4; ++i)
		{
			m[0 * 5 + i] = l[i];
			m[1 * 5 + i] = up[i];
			m[2 * 5 + i] = f[i];
			m[3 * 5 + i] = view[i * 5 + 3];
		}
	}

	vec4 resultA = Mat5_multiply(m, v, 1.0);
	vec4 result = Mat5_multiply(view, resultA, 1.0);

	gl_Position = result;
	instanceID = gl_InstanceID;
}