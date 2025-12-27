#version 330 core

layout(location = 0) out vec4 color;

in vec2 uv;

//uniform sampler2D prevPassGroup; // same as source for the first pass group, otherwise is the texture of the last pass of the prev group
uniform sampler2D p_group; // same as prevPassGroup // the more p there are the further back it goes i guess
//uniform sampler2D p_group_pass0;
//uniform vec4 p_group_pass0_size;
//uniform sampler2D p_group_passInd; // pass at the same index as this one of the previous group
//uniform vec4 p_group_passInd_size; // pass at the same index as this one of the previous group
//uniform sampler2D pass0;
//uniform vec4 pass0_size;
//uniform sampler2D prevPass;
//uniform vec4 prevPass_size;
uniform sampler2D source;
uniform sampler2D sourceDepth;
uniform vec4 sourceSize; // w, h, 1/w, 1/h

void main()
{
	vec4 s = texture(p_group, uv);
	color.rgb = 1.0 - s.rgb;
	color.a = s.a;
}
