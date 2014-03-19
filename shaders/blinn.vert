#version 330 core

in vec3 position;
in vec3 normal;
in vec4 tangent;
in vec2 texcoord;

out vec3 fragPosWorld;
out vec2 textureCoord;
out vec3 viewDirTan;
out vec3 lightDirTan;
out vec3 lightPosWorld;

uniform vec3 lightPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	vec3 bitangent = cross(normal, tangent.xyz) * tangent.w;
	vec3 lightDirWorld = lightPos - vec3(model * vec4(position, 1.0));
	vec3 cameraPosWorld = (inverse(view) * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
	vec3 cameraPosLoc = vec3(inverse(model) * vec4(cameraPosWorld, 1.0));
	vec3 viewDirLoc = normalize(cameraPosLoc - position);
	vec3 lightDirLoc = vec3(inverse(model) * vec4(lightDirWorld, 0.0));

	mat3 TBN = transpose(
		mat3(
			tangent.xyz,
			bitangent,
			normal
		)
	);
	lightDirTan = TBN * lightDirLoc;
	viewDirTan = TBN * viewDirLoc;

	gl_Position = proj * view * model * vec4(position, 1);
	fragPosWorld = (model * vec4(position, 1)).xyz;
	textureCoord = texcoord;
	lightPosWorld = lightPos;
}