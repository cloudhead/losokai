#include <GL/glew.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "text.h"

void rInitRenderer()
{
	// Use modern way of checking for feature availability
	glewExperimental = GL_TRUE;
	glewInit();

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_FRAMEBUFFER_SRGB);
}

void rClear()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void rDrawFrameTime(double ft)
{
	char str[128];

	sprintf(str, "frame time: %.2fms", ft);
	rDrawText2D(str, strlen(str), 10, 576, 16);
}

