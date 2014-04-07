#version 330 core

in vec3 position;
in vec3 normal;
in vec2 texcoord;
in int  padding[8];

out vec2 vTexcoord;
out vec3 vWorldPos;
out vec3 vNormal;
out vec4 _unused;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	vTexcoord = texcoord;
	vWorldPos = position;
	vNormal = normal;

	// XXX: This seems necessary because of the data layout,
	// which includes bones and weights (each 16 bytes).
	_unused = vec4(padding[0]);

	gl_Position = proj * view * model * vec4(position, 1);
}