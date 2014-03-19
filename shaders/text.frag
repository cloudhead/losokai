#version 330 core

in vec2 UV;

out vec4 fragColor;

uniform sampler2D sampler;

void main()
{
	fragColor = texture2D(sampler, UV);
}