#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>

#include "texture.h"
#include "linmath.h"
#include "shader.h"
#include "material.h"
#include "sds.h"

static bool rLoadMaterialTexture(struct material *m, const char *dir, const char *name, enum textureType type)
{
	char *path;
	struct texture *t;

	GLint format = rTextureFormat(type);

	path = sdscatprintf(sdsnew(dir), "/%s%s", name, rTextureExtension(type));

 	if ((t = rTextureFromPath(path, format)) == NULL) {
		return false;
 	}
 	m->textures[type] = t;
	t->index = type;
 	t->sampler = rNewSampler(GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR);
	rGenerateMipmap(t);

	sdsfree(path);

	return true;
}

struct material *rNewBasicMaterial(struct shader *s)
{
	struct material *m = malloc(sizeof(*m));

	memset(m, 0, sizeof(*m));
	m->shader = s;

	return m;
}

struct material *rNewMaterial(struct shader *s, const char *dir, const char *name)
{
	struct material *m = rNewBasicMaterial(s);

	memset(m->textures, 0, sizeof(m->textures));

	if (! rLoadMaterialTexture(m, dir, name, TEXTURE_TYPE_DIFFUSE))
		return NULL;
	if (! rLoadMaterialTexture(m, dir, name, TEXTURE_TYPE_SPECULAR))
		return NULL;
	if (! rLoadMaterialTexture(m, dir, name, TEXTURE_TYPE_NORMAL))
		return NULL;

	return m;
}

void rSetMaterialProperty4fv(struct material *m, const char *property, vec4 val)
{
	rUseShader(m->shader);
	rSetUniform4fv(m->shader, property, &val);
	rUseShader(0);
}

