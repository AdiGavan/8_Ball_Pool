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

// Uniform values for animation
uniform float timeElapsed;
uniform float angle;

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

	vec4 world_position2;
	world_position2 = Model * vec4(v_position, 1.0);
	
	if (timeElapsed != -1) {
		world_position2.z += (+0.3 + 0.3 * cos(timeElapsed)) * cos(angle);
		world_position2.y += (-0.3 - 0.3 * cos(timeElapsed)) * cos(17);

		world_position2.x += (+0.3 + 0.3 * cos(timeElapsed)) * sin(angle);
	}

	world_position =  vec3( Model * vec4(v_position,1) );
	world_normal = normalize( mat3(Model) * v_normal );

	gl_Position = Projection * View * world_position2;

}
