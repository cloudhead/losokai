#include <GL/glew.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "linmath.h"
#include "texture.h"
#include "shader.h"
#include "material.h"
#include "common.h"
#include "skeleton.h"
#include "mesh.h"
#include "cube.h"

void rDrawSkeleton(struct skeleton *sk, mat4 *transform)
{
	assert(sk);

	for (int i = 0; i < sk->nbones; i++) {
		struct bone *b = &sk->bones[i];

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		rDrawSphere(0.05f, 8, b->transform);
	}
}
