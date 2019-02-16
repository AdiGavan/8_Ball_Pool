#version 330
// "Copyright [2018] Gavan Adrian-George, 334CA"

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec3 v_texture;
layout(location = 3) in vec3 v_color;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

out vec3 frag_color;
out vec3 frag_normal;
out vec3 frag_texture;
out vec3 frag_position;
out vec3 world_position;
out vec3 world_normal;

void main()
{
	frag_color = v_color;
	frag_position = v_position;
	frag_texture = v_texture;
	frag_normal = v_normal;

	world_position =  vec3( Model * vec4(v_position,1) );
	world_normal = normalize( mat3(Model) * v_normal );

	gl_Position = Projection * View * Model * vec4(v_position, 1.0); 
}
