#include <stdlib.h>
#include <stdbool.h>
#include <GL/glew.h>

#include "linmath.h"
#include "light.h"
#include "mesh.h"
#include "cube.h"

enum visibility {
	VISIBILITY_HIDDEN,
	VISIBILITY_VISIBLE,
	VISIBILITY_DEBUG
};

struct mesh *LIGHT_SOURCE = NULL;

struct light *rNewLight()
{
	struct light *l = malloc(sizeof(*l));
	return l;
}

void rDrawLight(struct light *l)
{
	if (l->visibility == VISIBILITY_HIDDEN) {
		return;
	}

	if (l->visibility == VISIBILITY_DEBUG) { // Draw light source
		if (LIGHT_SOURCE == NULL) {
			LIGHT_SOURCE = rNewCube();
		}
		mat4 model = mat4scale(mat4identity(), 0.1f);
		model = mat4translate(model, l->pos);
		rMeshDraw(LIGHT_SOURCE, &model);
	}
}
