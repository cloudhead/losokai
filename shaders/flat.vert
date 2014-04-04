#version 330 core

in vec3  position;
in vec3  normal;
in int   bones[4];
in float weights[4];

flat   out vec3 fragPosWorld;
flat   out vec3 lightDir;
smooth out vec3 lightPosWorld;
flat   out vec3 vertexNormal;
flat   out vec4 vWeightColor;

uniform vec3 lightPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	vec3 lightDirWorld = lightPos - vec3(model * vec4(position, 1.0));
	vec3 lightDirLoc = vec3(inverse(model) * vec4(lightDirWorld, 0.0));

	vec4 boneColors[8] = vec4[](
		vec4(0.5, 0.1, 0.2, 1.0),
		vec4(0.2, 0.5, 0.1, 1.0),
		vec4(0.1, 0.1, 0.6, 1.0),
		vec4(0.2, 1.0, 0.6, 1.0),
		vec4(0.5, 0.2, 0.6, 1.0),
		vec4(0.5, 1.0, 0.2, 1.0),
		vec4(0.7, 0.3, 1.0, 1.0),
		vec4(0.5, 0.3, 0.7, 1.0)
	);

	vWeightColor = vec4(0);

	vWeightColor = mix(vWeightColor, boneColors[(bones[0])], weights[0]);
	vWeightColor = mix(vWeightColor, boneColors[(bones[1])], weights[1]);
	vWeightColor = mix(vWeightColor, boneColors[(bones[2])], weights[2]);
	vWeightColor = mix(vWeightColor, boneColors[(bones[3])], weights[3]);

	vertexNormal = normal;
	lightDir = normalize(lightDirLoc);
	fragPosWorld = (model * vec4(position, 1)).xyz;
	lightPosWorld = lightPos;

	gl_Position = proj * view * model * vec4(position, 1);
}