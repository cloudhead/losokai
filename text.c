#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <GL/glew.h>

#include "tga.h"
#include "text.h"
#include "linmath.h"
#include "shader.h"
#include "texture.h"

#define TEXT_SHADER_NAME "text"

static struct {
	GLuint          vbo;
	struct shader  *shader;
	struct texture *texture;
} TEXT2D;

bool rInitText2D(const char *path)
{
	struct shader *s = rGetShader(TEXT_SHADER_NAME);

	if (! s)
		return false;

	rUseShader(s);

	TEXT2D.texture = rTextureFromPath(path, GL_RGBA);
	TEXT2D.texture->sampler = rNewSampler(GL_LINEAR, false);
	TEXT2D.texture->uniform = glGetUniformLocation(s->handle, "sampler");
	TEXT2D.shader = s;

	rUseShader(0);
	glGenBuffers(1, &TEXT2D.vbo);

	return true;
}

// TODO(cloudhead): Use glMapBufferRange for better performance
void rDrawText2D(const char *str, int len, int x, int y, int size)
{
	int nvertices = len * 6;

	vec2 vertices[nvertices];
	vec2 *vertexp = vertices;
	vec2 uvs[nvertices];
	vec2 *uvp = uvs;

	for (int i = 0; i < len; i++) {
		vec2 nw = (vec2){x + i * size,        y + size};
		vec2 ne = (vec2){x + i * size + size, y + size};
		vec2 se = (vec2){x + i * size + size, y};
		vec2 sw = (vec2){x + i * size,        y};

		*vertexp++ = ne;
		*vertexp++ = sw;
		*vertexp++ = nw;

		*vertexp++ = sw;
		*vertexp++ = ne;
		*vertexp++ = se;

		char c = str[i];

		float uvx = (c % 16) / 16.0f;
		float uvy = (c / 16) / 16.0f;

		nw = (vec2){uvx,              1.0f - uvy};
		ne = (vec2){uvx + 1.0f/16.0f, 1.0f - uvy};
		se = (vec2){uvx + 1.0f/16.0f, 1.0f - (uvy + 1.0f/16.0f)};
		sw = (vec2){uvx,              1.0f - (uvy + 1.0f/16.0f)};

		*uvp++ = ne;
		*uvp++ = sw;
		*uvp++ = nw;

		*uvp++ = sw;
		*uvp++ = ne;
		*uvp++ = se;
	}
	rUseShader(TEXT2D.shader);
		glBindBuffer(GL_ARRAY_BUFFER, TEXT2D.vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(uvs), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(uvs), uvs);

		glActiveTexture(GL_TEXTURE0);
		glUniform1i(TEXT2D.texture->uniform, 0);
		glBindTexture(GL_TEXTURE_2D, TEXT2D.texture->handle);
		glBindSampler(0, TEXT2D.texture->sampler);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)sizeof(vertices));

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDrawArrays(GL_TRIANGLES, 0, nvertices);
		glDisable(GL_BLEND);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	rUseShader(0);
}
