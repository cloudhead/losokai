#version 330 core

uniform vec4 color;

in vec2  vTexcoord;
in vec3  vWorldPos;
in vec3  vNormal;

layout(location = 0) out vec3 fPosition;
layout(location = 1) out vec3 fDiffuse;
layout(location = 2) out vec3 fNormal;
layout(location = 3) out vec3 fTexcoord;

void main()
{
	fPosition = vWorldPos;
	fDiffuse = vec3(1);
	fNormal = normalize(vNormal);
	fTexcoord = vec3(vTexcoord.st, 0.0);
}
