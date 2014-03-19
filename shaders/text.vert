#version 330 core

layout(location = 0) in vec2 position; // Screen coordinates
layout(location = 1) in vec2 texcoord;

out vec2 UV;

void main()
{
	vec2 pos = position - vec2(400, 300);
	pos /= vec2(400, 300);
	gl_Position = vec4(pos, 0, 1);
	UV = texcoord;
}