#version 330
// "Copyright [2018] Gavan Adrian-George, 334CA"

in vec3 frag_color;
in vec3 frag_normal;
in vec3 frag_texture;
in vec3 frag_position;

layout(location = 0) out vec4 out_color;

void main()
{
	out_color = vec4(1, 0.5, 0, 1);
}