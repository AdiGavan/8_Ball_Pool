#version 330
// "Copyright [2018] Gavan Adrian-George, 334CA"

in vec3 world_position;
in vec3 world_normal;

uniform vec3 light_position;
uniform vec3 eye_position;
uniform float material_kd;
uniform float material_ks;
uniform int material_shininess;

vec3 object_color = vec3(0, 0.75, 0);

layout(location = 0) out vec4 out_color;

void main()
{
	vec3 N = normalize( world_normal );
	vec3 L = normalize( light_position - world_position );
	vec3 V = normalize( eye_position - world_position );
	vec3 H = normalize( L + V );

	float ambient_light = material_kd * 0.25;
	float diffuse_light = material_kd * max(dot(N,L), 0);
	float specular_light;

	if (diffuse_light > 0)
	{
		specular_light = material_ks * pow(max(dot(N, H), 0), material_shininess);
	} else {
		specular_light = 0;
	}

	float d = distance(light_position, eye_position);
	float attenuation_factor = 1 + 0.2 * d + 0.3 * d * d;

	float light = ambient_light + (diffuse_light + specular_light) / attenuation_factor;
	vec3 color = vec3(light * object_color);

	out_color = vec4(color, 1);
}