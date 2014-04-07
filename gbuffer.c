#include <stdlib.h>
#include <GL/glew.h>
#include <stdio.h>

#include "gbuffer.h"

#define elemsof(a) (sizeof(a) / sizeof(a[0]))

_Static_assert(sizeof(GLenum) == sizeof(int), "GLenum is the size of an integer");

struct gbuffer *rNewGbuffer(unsigned int width, unsigned int height, GLint format, GLint type)
{
	struct gbuffer *g = malloc(sizeof(*g));

	g->width = width;
	g->height = height;

	glGenFramebuffers(1, &g->fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, g->fbo);

	glGenTextures(elemsof(g->textures), g->textures);
	glGenTextures(1, &g->depth);

	// Color textures
	for (int i = 0; i < elemsof(g->textures); i++) {
		glBindTexture(GL_TEXTURE_2D, g->textures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, type, NULL);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, g->textures[i], 0);
	}

	// Depth texture
	glBindTexture(GL_TEXTURE_2D, g->depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g->depth, 0);

	GLenum drawbufs[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
	glDrawBuffers(elemsof(drawbufs), drawbufs);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		return NULL;
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	return g;
}

void rGbufferRBind(struct gbuffer *g)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, g->fbo);
}

void rGbufferWBind(struct gbuffer *g)
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, g->fbo);
}

void rGbufferSetRBuffer(struct gbuffer *g, enum gbufferTextureType t)
{
	glReadBuffer(GL_COLOR_ATTACHMENT0 + t);
}

void rGbufferBlit(struct gbuffer *g, enum gbufferTextureType t, int x, int y, int w, int h)
{
	rGbufferRBind(g);
	rGbufferSetRBuffer(g, t);
	glBlitFramebuffer(0, 0, g->width, g->height, x, y, x + w, y + h, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}
