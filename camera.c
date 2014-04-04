#include <assert.h>
#include <stdlib.h>

#include "linmath.h"
#include "camera.h"

struct camera *rNewCamera(vec3 pos, int width, int height, float fov, float znear, float zfar)
{
	struct camera *c = malloc(sizeof(*c));

	c->pos = pos;
	c->fov = fov;
	c->znear = znear;
	c->zfar = zfar;
	c->proj = mat4perspective(fov * PI/180, (float)width/(float)height, znear, zfar);

	return c;
}

void rCameraLookAt(struct camera *c, vec3 dir, vec3 up)
{
	c->view = mat4lookAt(c->pos, vec3add(c->pos, dir), up);
}

void rCameraMove(struct camera *c, vec3 dir)
{
	c->pos = vec3add(c->pos, dir);
}
