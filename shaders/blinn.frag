// TODO(cloudhead): When tonemapping is enabled, the diffuse channel needs to be brightened.
// TODO(cloudhead): Prefix input variables from vertex shader with 'v'.
#version 330 core

#define RENDER_TEXTURED      0 // Full shading, full diffuse and specular contribution
#define RENDER_SHADED        1 // Full shading, no diffuse or specular contribution
#define RENDER_SPECULAR      2 // Specular shading only
#define RENDER_FLAT_DIFFUSE  3 // Diffuse color only, flat shading
#define RENDER_FLAT_SPECULAR 4 // Specular color as diffuse, flat shading
#define RENDER_FLAT_NORMAL   5 // Normals as diffuse, flat shading

const float PI = 3.1415926535897932384626433832;

in vec3 fragPosWorld;
in vec2 textureCoord;
in vec3 viewDirTan;
in vec3 lightDirTan;
in vec3 lightPosWorld;
in vec3 vertexNormal;

out vec4 fragColor;

uniform bool      debugMode;
uniform bool      tonemapEnabled;
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

vec4 tonemap(in vec4 color)
{
	const float A = 0.22; // Shoulder strength
	const float B = 0.30; // Linear Strength
	const float C = 0.10; // Linear Angle
	const float D = 0.20; // Toe Strength
	const float E = 0.01; // Toe Numerator
	const float F = 0.30; // Toe Denominator

	return ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E/F;
}

vec4 computeFragColor(in vec4 diffuse, in vec4 specular, in float blinnTerm, in light l)
{
	float W = 11.2; // Whitepoint

	vec4 color = (diffuse * l.intensity * l.incidence * l.attenuation)  +
		   		 (specular * l.attenuation * blinnTerm) +
		   		 (diffuse * l.ambience);

	float exposure = 1.0f;

	if (tonemapEnabled) {
		return tonemap(exposure * color) / tonemap(vec4(W));
	} else {
		return color;
	}
}

float cookTorrance(in float roughness, in vec3 halfAngle, in vec3 normal, in vec3 viewDir, in vec3 lightDir)
{
	float F0 = 0.8; // Reflectance at normal incidence
	float m2 = roughness * roughness;

	float NdotH = dot(normal, halfAngle);
	float NdotV = dot(normal, viewDir);
	float NdotL = dot(normal, lightDir); // Same as NdotV?
	float VdotH = dot(viewDir, halfAngle);

	// Beckman's distribution function D
	float x = (NdotH * NdotH - 1.0) / (NdotH * NdotH * m2);
	float D = exp(x) / (4.0 * m2 * pow(NdotH, 4));

	// Fresnel term F
	float F = pow(1.0 - NdotV, 5.0);
	F *= (1.0 - F0);
	F += F0;

	// Self-shadowing term G
	float X = 2.0 * NdotH / VdotH;
	float G = min(1.0, min(X * NdotL, X * NdotV));

	return (D * F * G) / (NdotV * PI);
}

float gaussian(in float roughness, in vec3 halfAngle, in vec3 normal)
{
	float angleNormalHalf = acos(dot(halfAngle, normal));
	float exponent = angleNormalHalf / roughness;
	exponent = -(exponent * exponent);
	return exp(exponent);
}

float blinn(in float roughness, in vec3 halfAngle, in vec3 normal)
{
	float blinnTerm = clamp(dot(normal, halfAngle), 0, 1);
	blinnTerm = pow(blinnTerm, roughness);
	return blinnTerm;
}

void main()
{
	light l;

	l.intensity = vec4(1.0, 1.0, 0.9, 0.0) * 30;
	l.ambience = vec4(0.05, 0.05, 0.08, 0.0) * 3;

	const float roughness = 0.3;
	const float specularIntensity = 15.0;

	vec4 grey = vec4(0.1, 0.1, 0.1, 1.0);
	vec2 t = vec2(textureCoord.s, 1.0 - textureCoord.t);

	// Sample textures
	vec4 diffuse = texture(diffuseSampler, t);
	vec4 normalColor = texture(normalSampler, t);
	vec4 specularColor = texture(specularSampler, textureCoord);

	// Convert RGB values to [-1, 1] range
	vec3 normal = normalize(normalColor.rgb * 2.0 - 1.0);

	vec4 specular = vec4(specularColor.rgb * specularIntensity, 1.0);

	float lightDistance = length(lightPosWorld - fragPosWorld);

	l.attenuation = 1.0 / (1.0 + lightDistance * lightDistance);
	l.incidence = clamp(dot(lightDirTan, normal), 0, 1);

	vec3 halfAngle = normalize(lightDirTan + viewDirTan);
	float specTerm = cookTorrance(roughness, halfAngle, normal, viewDirTan, lightDirTan);

	if (l.incidence == 0.0) {
		specTerm = 0.0;
	}

	switch (renderMode) {
		case RENDER_TEXTURED:      fragColor = computeFragColor(diffuse, specular, specTerm, l); break;
		case RENDER_SHADED:        fragColor = computeFragColor(grey, vec4(1.0), specTerm, l); break;
		case RENDER_SPECULAR:      fragColor = computeFragColor(vec4(0), specular, specTerm, l); break;
		case RENDER_FLAT_DIFFUSE:  fragColor = diffuse; break;
		case RENDER_FLAT_NORMAL:   fragColor = normalColor; break;
		case RENDER_FLAT_SPECULAR: fragColor = specularColor; break;
	}
}

