#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>

#include "linmath.h"
#include "shader.h"
#include "texture.h"
#include "common.h"
#include "mesh.h"
#include "material.h"

static const vec3 VERTICES[] = {
	(vec3){ 0.5f,  0.5f,  0.5f},
	(vec3){-0.5f, -0.5f,  0.5f},
	(vec3){-0.5f,  0.5f, -0.5f},
	(vec3){ 0.5f, -0.5f, -0.5f},
	(vec3){-0.5f, -0.5f, -0.5f},
	(vec3){ 0.5f,  0.5f, -0.5f},
	(vec3){ 0.5f, -0.5f,  0.5f},
	(vec3){-0.5f,  0.5f,  0.5f}
};

static const unsigned int ELEMENTS[] = {
	0, 1, 2,
	1, 0, 3,
	2, 3, 0,
	3, 2, 1,
	5, 4, 6,
	4, 5, 7,
	7, 6, 4,
	6, 7, 5
};

struct mesh *rNewCube()
{
	int nverts = 8;
	int nfaces = 24/3;

	struct material *mat = rNewBasicMaterial(rGetShader("constant"));

	unsigned int *faces = malloc(24 * sizeof(unsigned int));
	memcpy(faces, ELEMENTS, sizeof(ELEMENTS));

	struct vertex *verts = malloc(nverts * sizeof(struct vertex));

	for (int i = 0; i < nverts; i++) {
		verts[i].pos = VERTICES[i];
	}
	return rNewMesh("cube", mat, nverts, verts, nfaces, faces, NULL);
}

struct mesh *rNewSphere(float radius, int reso)
{
	struct material *mat = rNewBasicMaterial(rGetShader("constant"));
	rSetMaterialProperty4fv(mat, "color", (vec4){1.0f, 0.0f, 0.0f, 0.5f});

	int nverts = 6 * reso * reso;
	struct vertex *verts = malloc(nverts * sizeof(struct vertex));
	struct vertex *vptr = verts;

	for (int w = 0; w < reso; w++) {
		for (int h = -reso/2; h < reso/2; h++) {
			float inc1 = w / (float)reso* 2 * PI;
			float inc2 = (w + 1) / (float)reso * 2 * PI;
			float inc3 = h / (float)reso * PI;
			float inc4 = (h + 1) / (float)reso * PI;

			float x1 = sinf(inc1);
			float y1 = cosf(inc1);
			float x2 = sinf(inc2);
			float y2 = cosf(inc2);

			float radius1 = radius * cosf(inc3);
			float radius2 = radius * cosf(inc4);

			float z1 = radius * sinf(inc3);
			float z2 = radius * sinf(inc4);

			(*vptr++).pos = (vec3){radius1 * x1, z1, radius1 * y1};
			(*vptr++).pos = (vec3){radius1 * x2, z1, radius1 * y2};
			(*vptr++).pos = (vec3){radius2 * x2, z2, radius2 * y2};
			(*vptr++).pos = (vec3){radius1 * x1, z1, radius1 * y1};
			(*vptr++).pos = (vec3){radius2 * x2, z2, radius2 * y2};
			(*vptr++).pos = (vec3){radius2 * x1, z2, radius2 * y1};
		}
	}
	return rNewMesh("sphere", mat, nverts, verts, 0, NULL, NULL);
}

//TODO(cloudhead): Don't recreate mesh every time.
void rDrawSphere(float radius, int reso, mat4 transform)
{
	struct mesh *m = rNewSphere(radius, reso);
	rDrawMesh(m, &transform);
	meshFree(m);
}
