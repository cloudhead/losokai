//
// model.c
// model loading & drawing
//
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <dirent.h>
#include <sys/types.h>
#include <GL/glew.h>

#include "linmath.h"
#include "shader.h"
#include "texture.h"
#include "mesh.h"
#include "model.h"
#include "sds.h"
#include "material.h"

#define MAGIC_NUMBER 236

char *strdup(const char *s);

const char *textureSamplerNames[] = {
	[TEXTURE_TYPE_DIFFUSE] = "diffuseSampler",
	[TEXTURE_TYPE_NORMAL] = "normalSampler",
	[TEXTURE_TYPE_SPECULAR] = "specularSampler"
};

void rFreeMdl(struct model *m)
{
	for (int i = 0; i < m->nmeshes; i++) {
		meshFree(m->meshes[i]);
	}
	free(m->meshes);
	free(m);
}

static bool rLoadMdlMeshMaterial(struct mesh *m, const char *shader, const char *dir)
{
	char *path = sdscat(sdsnew(dir), "/textures");
	struct shader *s = rGetShader(shader);

	if (! s) {
		fprintf(stderr, "couldn't find shader '%s'.\n", shader);
		return false;
	}
	if (! (m->material = rNewMaterial(s, path, m->name))) {
		m->material = rNewBasicMaterial(s);
	}
	sdsfree(path);

	return true;
}

// File format:
//
//   COMMAND <space> '"' VALUE '"' '\n'
//
static bool rLoadMdlMetadata(struct model *mdl, const char *dir)
{
	char line[512], cmd[32], val[256];
	char *path = sdscatprintf(sdsnew(dir), "/%s.meta", mdl->name);

	FILE *fp = fopen(path, "r");

	while (fgets(line, sizeof(line), fp) != NULL) {
		char *lineptr = line, *cmdptr = cmd, *valptr = val;

		while (*lineptr != ' ') *cmdptr++ = *lineptr++;

		*cmdptr = '\0'; lineptr++; // skip ' '

		if (*lineptr++ != '"') return false;
		while (*lineptr != '"') *valptr++ = *lineptr++;

		*valptr = '\0';

		if (*lineptr++ != '"')  return false;
		if (*lineptr++ != '\n') return false;
	}
	return true;
}

static bool rLoadMdlMeshes(struct model *mdl, const char *dir)
{
	char *path = sdscatprintf(sdsnew(dir), "/%s.mesh", mdl->name);
	FILE *fp = fopen(path, "rb");

	rLoadMdlMetadata(mdl, dir);

	if (!fp) {
		fprintf(stderr, "couldn't fopen %s\n", path);
		return false;
	}

	unsigned char magic = 0;
	fread(&magic, 1, 1, fp);
	if (magic != MAGIC_NUMBER) {
		fprintf(stderr, "file isn't a lourland model.\n");
		return false;
	}

	fread(&mdl->nmeshes, 4, 1, fp);
	mdl->meshes = malloc(mdl->nmeshes * sizeof(struct mesh *));

	for (int i = 0; i < mdl->nmeshes; i++) {
		struct mesh *m = mdl->meshes[i] = rNewMesh(NULL);

		{ // Read mesh name
			int len = fgetc(fp);

			if (len > 0) {
				m->name = malloc(len + 1);
				fread(m->name, len, 1, fp);
				m->name[len] = '\0';
			} else {
				m->name = strdup(mdl->name);
			}
		}

		{ // Read shader name and load material
			int len = fgetc(fp);
			char shader[len + 1];

			if (len > 0) {
				fread(shader, len, 1, fp);
				shader[len] = '\0';
			} else {
				sprintf(shader, "default");
			}
			if (! rLoadMdlMeshMaterial(m, shader, dir)) {
				fprintf(stderr, "couldn't load mesh material.\n");
				m->isVisible = false;
			}
		}

		unsigned int material;
		fread(&material, sizeof(material), 1, fp);

		m->vertices = NULL;
		m->faces = NULL;
		m->nfaces = 0;
		m->nvertices = 0;

		fread(&m->nvertices, 4, 1, fp);
		assert(m->nvertices > 0);
		m->vertices = malloc(m->nvertices * sizeof(struct vertex));

		for (int j = 0; j < m->nvertices; j++) {
			fread(&m->vertices[j].pos, sizeof(float), 3, fp);
			fread(&m->vertices[j].normal, sizeof(float), 3, fp);
			fread(&m->vertices[j].tangent, sizeof(float), 4, fp);
			fread(&m->vertices[j].uv, sizeof(float), 2, fp);
		}
		fread(&m->nfaces, 4, 1, fp);
		assert(m->nfaces > 0);
		m->faces = malloc(m->nfaces * sizeof(unsigned int) * 3);

		for (int j = 0; j < m->nfaces * 3; j++) {
			fread(&m->faces[j], sizeof(unsigned int), 1, fp);
		}
		meshInit(m);
	}
	sdsfree(path);

	return true;
}

void rDrawMdl(struct model *mdl)
{
	mat4 model = mat4identity();

	model = mat4scale(model, 0.01f);

	for (int i = 0; i < mdl->nmeshes; i++) {
		if (! mdl->meshes[i]->isVisible)
			continue;

		GLuint program = mdl->meshes[i]->material->shader->handle;

		glUseProgram(program);

		// TODO(cloudhead): This value is the same for all meshes, find a way to avoid
		// respecifying it for every mesh.
		GLint uniModel = glGetUniformLocation(program, "model");
		glUniformMatrix4fv(uniModel, 1, GL_FALSE, (float *)model.cols);

		glBindVertexArray(mdl->meshes[i]->vao);
		glBindBuffer(GL_ARRAY_BUFFER, mdl->meshes[i]->vbo); // Make it the active object

		GLint posAttrib = glGetAttribLocation(program, "position");
		glEnableVertexAttribArray(posAttrib);
		glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), 0);

		// TODO: Use GL_INT_2_10_10_10_REV instead of GL_FLOAT
		GLint normAttrib = glGetAttribLocation(program, "normal");
		glEnableVertexAttribArray(normAttrib);
		glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_TRUE, sizeof(struct vertex), (void *)offsetof(struct vertex, normal));

		GLint tangentAttrib = glGetAttribLocation(program, "tangent");
		glEnableVertexAttribArray(tangentAttrib);
		glVertexAttribPointer(tangentAttrib, 4, GL_FLOAT, GL_TRUE, sizeof(struct vertex), (void *)offsetof(struct vertex, tangent));

		GLint uvAttrib = glGetAttribLocation(program, "texcoord");
		glEnableVertexAttribArray(uvAttrib);
		glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void *)offsetof(struct vertex, uv));

		// Bind textures
		for (int j = 0; j < TEXTURE_TYPES; j++) {
			struct texture *t = mdl->meshes[i]->material->textures[j];

			if (t == NULL)
				continue;

			glActiveTexture(GL_TEXTURE0 + t->index);

			GLint unif = glGetUniformLocation(program, textureSamplerNames[j]);
			// assert(unif > 0);

			glUniform1i(unif, t->index);
			glBindTexture(GL_TEXTURE_2D, t->handle);
			glBindSampler(t->index, t->sampler);
			glActiveTexture(GL_TEXTURE0);
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mdl->meshes[i]->ebo);
		glDrawElements(GL_TRIANGLES, mdl->meshes[i]->nfaces * 3, GL_UNSIGNED_INT, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glUseProgram(0);
	}
}

struct model *rOpenMdl(const char *path)
{
	struct model *mdl = malloc(sizeof(*mdl));

	mdl->name = strdup(path);
	mdl->nmeshes = 0;
	mdl->meshes = NULL;

	if (! rLoadMdlMeshes(mdl, path))
		return NULL;

	return mdl;
}

