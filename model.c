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
#include "common.h"
#include "mesh.h"
#include "model.h"
#include "sds.h"
#include "material.h"
#include "util.h"
#include "skeleton.h"

static const int  MAGIC_NUMBER  = 236;
static const char ASSET_DIR[]   = "assets";
static const char TEXTURE_DIR[] = "textures";
static const char META_EXT[]    = ".meta";
static const char MESH_EXT[]    = ".mesh";

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

static struct material *rLoadMdlMaterial(char *name, const char *shader, const char *dir)
{
	const char *parts[] = {ASSET_DIR, dir, TEXTURE_DIR};
	char *path = sdsjoin((char **)parts, 3, "/", 1);

	struct shader *s = rGetShader(shader);
	struct material *mat;

	if (! s) {
		fprintf(stderr, "couldn't find shader '%s'.\n", shader);
		return NULL;
	}
	if (! (mat = rNewMaterial(s, path, name))) {
		mat = rNewBasicMaterial(s);
	}
	sdsfree(path);

	return mat;
}

// File format:
//
//   COMMAND <space> '"' VALUE '"' '\n'
//
static bool rLoadMdlMetadata(struct model *mdl, const char *dir)
{
	const char *parts[] = {ASSET_DIR, (char *)dir, (char *)mdl->name};
	char *path = sdscat(sdsjoin((char **)parts, 3, "/", 1), META_EXT);
	char line[512], cmd[32], val[256];

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
	const char *parts[] = {ASSET_DIR, dir, mdl->name};
	char *path = sdscat(sdsjoin((char **)parts, 3, "/", 1), MESH_EXT);

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
		unsigned int material = 0;
		struct material *mat = NULL;
		struct vertex *vertices = NULL;
		unsigned int *faces = NULL;
		size_t nfaces = 0;
		size_t nvertices = 0;

		// Read mesh name
		int len = fgetc(fp);
		char name[len + 1];

		if (len > 0) {
			fread(name, len, 1, fp);
			name[len] = '\0';
		} else {
			strcpy(name, mdl->name);
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
			if (! (mat = rLoadMdlMaterial(name, shader, dir))) {
				fprintf(stderr, "couldn't load mesh material.\n");
			}
		}

		// XXX: Unused
		fread(&material, sizeof(material), 1, fp);

		struct skeleton *sk = malloc(sizeof(*sk));

		sk->bones = NULL;
		sk->nbones = 0;

		// Read bones
		fread(&sk->nbones, 4, 1, fp);

		sk->bones = malloc(sk->nbones * sizeof(struct bone));

		for (int j = 0; j < sk->nbones; j++) {
			freadstr(&sk->bones[j].name, fp);
			fread(&sk->bones[j].offset, sizeof(mat4), 1, fp);
			fread(&sk->bones[j].transform, sizeof(mat4), 1, fp);
			fread(&sk->bones[j].parentId, 4, 1, fp);
			sk->bones[j].length = 0.1f;
		}

		// Read vertices
		fread(&nvertices, 4, 1, fp);
		assert(nvertices > 0);
		vertices = malloc(nvertices * sizeof(struct vertex));
		fread(vertices, sizeof(struct vertex), nvertices, fp);

		// Read faces
		fread(&nfaces, 4, 1, fp);
		assert(nfaces > 0);
		faces = malloc(nfaces * sizeof(unsigned int) * 3);
		fread(faces, sizeof(unsigned int), nfaces * 3, fp);

		mdl->meshes[i] = rNewMesh(name, mat, nvertices, vertices, nfaces, faces, sk);
	}
	sdsfree(path);

	return true;
}

void rDrawMdl(struct model *mdl)
{
	mat4 model = mat4identity();

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

		GLint bonesAttrib = glGetAttribLocation(program, "bones");
		glEnableVertexAttribArray(bonesAttrib);
		glVertexAttribIPointer(bonesAttrib, 4, GL_INT, sizeof(struct vertex), (void *)offsetof(struct vertex, bones));

		GLint weightsAttrib = glGetAttribLocation(program, "weights");
		glEnableVertexAttribArray(weightsAttrib);
		glVertexAttribPointer(weightsAttrib, 4, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void *)offsetof(struct vertex, weights));

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
		glBindVertexArray(0);

		glDisable(GL_DEPTH_TEST);
		rDrawSkeleton(mdl->meshes[i]->skeleton, &model);
		glEnable(GL_DEPTH_TEST);
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

