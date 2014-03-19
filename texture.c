#include <stdbool.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "texture.h"
#include "tga.h"

static const char *TextureExtensions[] = {
	[TEXTURE_TYPE_DIFFUSE] = "_d.tga",
	[TEXTURE_TYPE_NORMAL] = "_n.tga",
	[TEXTURE_TYPE_SPECULAR] = "_s.tga"
};

static GLint TextureFormats[] = {
	[TEXTURE_TYPE_DIFFUSE] = GL_RGBA,//GL_SRGB8_ALPHA8,
	[TEXTURE_TYPE_NORMAL] = GL_RGBA,
	[TEXTURE_TYPE_SPECULAR] = GL_SRGB8_ALPHA8
};

struct texture *rTextureFromPath(const char *path, GLint format)
{
	struct tga t;
	struct texture *tx;

	if (! tgaDecode(&t, path)) {
		return NULL;
	}
	tx = rNewTexture(t.data, t.width, t.height, format);
	tgaFreeImageData(&t);

	return tx;
}

GLuint rNewSampler(GLuint minFilter, GLuint magFilter)
{
	GLuint sampler;
	glGenSamplers(1, &sampler);

	// Filter to use when texture is minified and magnified.
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, minFilter);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, magFilter);

	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return sampler;
}

void rGenerateMipmap(struct texture *t)
{
	glBindTexture(GL_TEXTURE_2D, t->handle);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

struct texture *rNewTexture(void *pixels, int w, int h, GLint format)
{
	struct texture *t = malloc(sizeof(*t));

	t->index = 0;
	t->sampler = -1;
	t->uniform = -1;

	glGenTextures(1, &t->handle);
	glBindTexture(GL_TEXTURE_2D, t->handle);
	glTexImage2D(
		GL_TEXTURE_2D,
		0, // Mipmap level
		format, // Internal texel format
		w, h, // Width & height
		0, // Should always be 0
		GL_BGRA, // Texel format of array
		GL_UNSIGNED_BYTE, // Data type of the components
		pixels // Data
	);
	glBindTexture(GL_TEXTURE_2D, 0);

	return t;
}

const char *rTextureExtension(enum textureType t)
{
	return TextureExtensions[t];
}

GLint rTextureFormat(enum textureType t)
{
	return TextureFormats[t];
}
