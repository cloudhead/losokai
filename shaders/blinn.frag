#version 330 core

#define RENDER_TEXTURED      0 // Full shading, full diffuse and specular contribution
#define RENDER_SHADED        1 // Full shading, no diffuse or specular contribution
#define RENDER_FLAT_DIFFUSE  2 // Diffuse color only, flat shading
#define RENDER_FLAT_SPECULAR 3 // Specular color as diffuse, flat shading
#define RENDER_FLAT_NORMAL   4 // Normals as diffuse, flat shading

in vec3 fragPosWorld;
in vec2 textureCoord;
in vec3 viewDirTan;
in vec3 lightDirTan;
in vec3 lightPosWorld;

out vec4 fragColor;

uniform int       renderMode;
uniform sampler2D diffuseSampler;
uniform sampler2D specularSampler;
uniform sampler2D normalSampler;

struct light {
	vec4 intensity;
	vec4 ambience;
	float attenuation;
	float incidence;
};

vec4 computeFragColor(in vec4 diffuse, in float specular, in float blinnTerm, in light l)
{
	return (diffuse * l.intensity * l.incidence * l.attenuation)  +
		   (specular * l.attenuation * blinnTerm) +
		   (diffuse * l.ambience);
}

void main()
{
	light l;

	l.intensity = vec4(1.0, 1.0, 0.9, 0.0) * 30;
	l.ambience = vec4(0.05, 0.05, 0.08, 0.0) * 3;

	vec4 grey = vec4(0.1, 0.1, 0.1, 1.0);
	vec2 t = vec2(textureCoord.s, 1.0 - textureCoord.t);

	// Sample textures
	vec4 diffuse = texture(diffuseSampler, t);
	vec4 normalColor = texture(normalSampler, t);
	vec4 specularColor = texture(specularSampler, textureCoord);

	// Convert RGB values to [-1, 1] range
	vec3 normal = normalize(normalColor.rgb * 2.0 - 1.0);

	float specular = specularColor.r * 10;
	float shininess = 10;

	float lightDistance = length(lightPosWorld - fragPosWorld);
	vec3 lightDir = normalize(lightDirTan);

	l.attenuation = 1.0 / (1.0 + lightDistance * lightDistance);
	l.incidence = clamp(dot(lightDir, normal), 0, 1);

	vec3 halfAngle = normalize(lightDirTan + viewDirTan);
	float blinnTerm = clamp(dot(normal, halfAngle), 0, 1);
	blinnTerm = l.incidence != 0.0 ? blinnTerm : 0.0;
	blinnTerm = pow(blinnTerm, shininess);

	switch (renderMode) {
		case RENDER_TEXTURED:      fragColor = computeFragColor(diffuse, specular, blinnTerm, l); break;
		case RENDER_SHADED:        fragColor = computeFragColor(grey, 0.5, blinnTerm, l); break;
		case RENDER_FLAT_DIFFUSE:  fragColor = diffuse; break;
		case RENDER_FLAT_NORMAL:   fragColor = normalColor; break;
		case RENDER_FLAT_SPECULAR: fragColor = specularColor; break;
	}
}

