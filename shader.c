#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <stdbool.h>
#include <assert.h>

#include "util.h"
#include "linmath.h"
#include "shader.h"
#include "dict.h"

#define elems(a) (sizeof(a) / sizeof(a[0]))

static dict_t SHADERS;

//
// Display compilation errors from the OpenGL shader compiler
//
static void printlog(GLuint shader)
{
	char log[512];
	glGetShaderInfoLog(shader, sizeof(log), NULL, log);
	fprintf(stderr, "%s", log);
}

void rUseShader(struct shader *s)
{
	if (s) { glUseProgram(s->handle); }
	else   { glUseProgram(0); }
}

struct shader *rGetShader(const char *name)
{
	return dictLookup(SHADERS, name);
}

void rDeleteShader(struct shader *s)
{
	glDeleteProgram(s->handle);
	free(s);
}

//
// Compile the shader from file `filename`
//
struct shader *rShaderFromPath(const char *filename, GLenum type)
{
	const GLchar *source = readfile(filename);

	if (source == NULL) {
		fprintf(stderr, "error opening %s", filename);
		return NULL;
	}
	GLuint handle = glCreateShader(type);
	struct shader *s = rNewShader(filename, handle);

	glShaderSource(s->handle, 1, &source, NULL);
	glCompileShader(s->handle);

	free((char *)source);

	GLint status;
	glGetShaderiv(s->handle, GL_COMPILE_STATUS, &status);

	if (status != GL_TRUE) {
		fprintf(stderr, "%s: ", filename);
		printlog(s->handle);
		glDeleteShader(s->handle);
		return NULL;
	}
	return s;
}

struct shader *rNewShader(const char *name, GLuint handle)
{
	struct shader *s = malloc(sizeof(*s) + strlen(name) + 1);
	strcpy(s->name, name);
	s->handle = handle;
	s->uniforms = dict(NULL);

	return s;
}

bool rLoadShader(const char *name, const char *vertpath, const char *fragpath)
{
	struct shader *vert, *frag;

	if (! (vert = rShaderFromPath(vertpath, GL_VERTEX_SHADER)))
		return false;

	if (! (frag = rShaderFromPath(fragpath, GL_FRAGMENT_SHADER)))
		return false;

	GLuint program = glCreateProgram();
	glAttachShader(program, vert->handle);
	glAttachShader(program, frag->handle);

	// Not currently necessary because only one buffer
	glBindFragDataLocation(program, 0, "fragColor");
	glLinkProgram(program);

	dictInsert(SHADERS, name, rNewShader(name, program));

	return true;
}

bool rLoadShaders(struct shaderSource *sources)
{
	SHADERS = dict(NULL);

	for (struct shaderSource *s = sources; s->name != NULL; s++) {
		if (! rLoadShader(s->name, s->vert, s->frag)) {
			return false;
		}
	}
	return true;
}

void rUnloadShader(const char *name)
{
	struct shader *s = rGetShader(name);
	rDeleteShader(s);
}

void rUnloadShaders(struct shaderSource *sources)
{
	for (struct shaderSource *s = sources; s->name != NULL; s++) {
		rUnloadShader(s->name);
	}
	dictFree(SHADERS);
}

// TODO(cloudhead): Optimize lookup with a cache.
void rSetUniformMatrix4fv(struct shader *s, const char *name, mat4 *m)
{
	GLint loc;

	if ((loc = glGetUniformLocation(s->handle, name)) == -1) {
		// TODO(cloudhead): Log error.
		return;
	}
	glUniformMatrix4fv(loc, 1, GL_FALSE, (float *)m->cols);
}

// TODO(cloudhead): Optimize lookup with a cache.
void rSetUniform3fv(struct shader *s, const char *name, vec3 *v)
{
	GLint loc;

	if ((loc = glGetUniformLocation(s->handle, name)) == -1) {
		// TODO(cloudhead): Log error.
		return;
	}
	glUniform3fv(loc, 1, (float *)v);
}

// TODO(cloudhead): Optimize lookup with a cache.
void rSetUniform4fv(struct shader *s, const char *name, vec4 *v)
{
	GLint loc;

	if ((loc = glGetUniformLocation(s->handle, name)) == -1) {
		// TODO(cloudhead): Log error.
		fprintf(stderr, "couldn't get uniform location for '%s'\n", name);
		return;
	}
	glUniform4fv(loc, 1, (float *)v);
}

// TODO(cloudhead): Optimize lookup with a cache.
void rSetUniform1i(struct shader *s, const char *name, GLint i)
{
	GLint loc;

	if ((loc = glGetUniformLocation(s->handle, name)) == -1) {
		// TODO(cloudhead): Log error.
		return;
	}
	glUniform1i(loc, i);
}
