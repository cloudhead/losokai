#include <stdlib.h>
#include <stdbool.h>
#include <GL/glew.h>

#include "linmath.h"
#include "texture.h"
#include "shader.h"
#include "material.h"
#include "common.h"
#include "skeleton.h"
#include "mesh.h"

char *strdup(const char *);

static void rInitMesh(struct mesh *m)
{
	// Store the following attrib properties in the vao
	glGenVertexArrays(1, &m->vao);
	glBindVertexArray(m->vao);

	glGenBuffers(1, &m->vbo); // Create a VBO
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo); // Make it the active object
	glBufferData(GL_ARRAY_BUFFER, m->nvertices * sizeof(struct vertex), (GLfloat *)m->vertices, GL_STATIC_DRAW); // Copy vertex data to it

	if (m->faces) {
		glGenBuffers(1, &m->ebo); // Create a EBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->nfaces * 3 * sizeof(unsigned int), m->faces, GL_STATIC_DRAW);
	}
	m->isVisible = true;

	glBindVertexArray(0);

	// GL_STATIC_DRAW: The vertex data will be uploaded once and drawn many times (e.g. the world).
	// GL_DYNAMIC_DRAW: The vertex data will be changed from time to time, but drawn many times more than that.
	// GL_STREAM_DRAW: The vertex data will change almost every time it's drawn (e.g. user interface).
}

void meshFree(struct mesh *m)
{
	glDeleteBuffers(1, &m->vbo);

	if (m->ebo)
		glDeleteBuffers(1, &m->ebo);
	if (m->vao)
		glDeleteVertexArrays(1, &m->vao);

	free(m->name);
	free(m);
}

struct mesh *rNewMesh(name, mat, nverts, verts, nfaces, faces, sk)
	const char      *name;
	struct material *mat;
	size_t          nverts;
	struct vertex   *verts;
	size_t          nfaces;
	unsigned int    *faces;
	struct skeleton *sk;
{
	struct mesh *m = malloc(sizeof(*m));
	m->name = name == NULL ? NULL : strdup(name);
	m->skeleton = sk;
	m->material = mat;
	m->nvertices = nverts;
	m->vertices = verts;
	m->nfaces = nfaces;
	m->faces = faces;
	m->ebo = 0;
	m->vbo = 0;
	m->vao = 0;

	rInitMesh(m);

	return m;
}

// TODO(cloudhead): Use shader API.
// TODO(cloudhead): rDrawMdl should use this function.
void rDrawMesh(struct mesh *m, mat4 *transform)
{
	GLuint program = m->material->shader->handle;
	glUseProgram(program);
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo); // Make it the active object

	glBindVertexArray(m->vao);

	GLint uniModel = glGetUniformLocation(program, "model");
	glUniformMatrix4fv(uniModel, 1, GL_FALSE, (float *)transform->cols);

	GLint posAttrib = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), 0);

	if (m->faces) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ebo);
		glDrawElements(GL_TRIANGLES, m->nfaces * 3, GL_UNSIGNED_INT, 0);
	} else {
		glDrawArrays(GL_TRIANGLES, 0, m->nvertices);
	}
	glUseProgram(0);
	glBindVertexArray(0);
}
