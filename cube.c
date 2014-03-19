#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>

#include "linmath.h"
#include "shader.h"
#include "texture.h"
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
    struct mesh *m = rNewMesh("cube");

    m->nfaces = 24/3;
    m->nvertices = 8;
    m->faces = malloc(24 * sizeof(unsigned int));
    m->vertices = malloc(m->nvertices * sizeof(struct vertex));

    for (int i = 0; i < m->nvertices; i++) {
        m->vertices[i].pos = VERTICES[i];
    }
    memcpy(m->faces, ELEMENTS, sizeof(ELEMENTS));
    m->material = rNewBasicMaterial(rGetShader("flat"));
    meshInit(m);
    return m;
}
