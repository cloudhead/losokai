// TODO(cloudhead): Prefix input variables from vertex shader with 'v'.
#version 330 core

flat   in vec3 fragPosWorld;
flat   in vec3 lightDir;
smooth in vec3 lightPosWorld;
flat   in vec3 vertexNormal;
flat   in vec4 vWeightColor;

out vec4 fragColor;

void main()
{
	vec4 intensity = vec4(1.0, 1.0, 0.9, 0.0) * 30;
	vec4 ambience = vec4(0.05, 0.05, 0.08, 0.0) * 3;
	vec4 diffuse = vec4(0.1, 0.1, 0.1, 1.0);

	float lightDistance = length(lightPosWorld - fragPosWorld);
	float attenuation = 1.0 / (1.0 + lightDistance * lightDistance);
	float incidence = clamp(dot(lightDir, vertexNormal), 0, 1);

	fragColor = (diffuse * intensity * incidence * attenuation) + (diffuse * ambience);
}
