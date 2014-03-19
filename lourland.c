#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <GLFW/glfw3.h>

#include "linmath.h"
#include "util.h"
#include "texture.h"
#include "mesh.h"
#include "model.h"
#include "shader.h"
#include "light.h"
#include "camera.h"
#include "command.h"
#include "network.h"
#include "renderer.h"
#include "text.h"

static const int WIDTH = 800;
static const int HEIGHT = 600;
static const int CMD_PORT = 8000;

static struct shaderSource SHADER_SOURCES[] = {
	{"default", "shaders/blinn.vert",  "shaders/blinn.frag"},
	{"flat",    "shaders/mvp.vert",    "shaders/flat.frag"},
	{"text",    "shaders/text.vert",   "shaders/text.frag"},
	{NULL,      NULL,                  NULL}
};

_Static_assert(sizeof(vec4) == sizeof(float) * 4, "vec4 is tightly packed");

static void errorCallback(int error, const char *description)
{
	fprintf(stderr, "GLFW Error: %s\n", description);
}

static void keyCallback(GLFWwindow *win, int key, int scan, int action, int mods)
{
	if (action != GLFW_PRESS)
		return;

	int *renderMode = glfwGetWindowUserPointer(win);

	if (key == GLFW_KEY_R) {
		(*renderMode)++;
		*renderMode %= 5;
	}
}

static void cursorEnterCallback(GLFWwindow *win, int entered)
{
	if (entered) {
		glfwSetCursorPos(win, WIDTH/2.0f, HEIGHT/2.0f);
	}
}

int main(int argc, char *argv[])
{
	GLFWwindow *win;

	glfwSetErrorCallback(errorCallback);

	if (! glfwInit())
		exit(1);

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);

	// Require OpenGL 3.3 or later
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_SAMPLES, 16);

	// Only support new core functionality
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	if ((win = glfwCreateWindow(WIDTH, HEIGHT, "lourland", NULL, NULL)) == NULL) {
		glfwTerminate();
		fatalf("error creating window\n");
	}

	glfwMakeContextCurrent(win);
	glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	glfwSetCursorEnterCallback(win, cursorEnterCallback);
	glfwSetKeyCallback(win, keyCallback);

	rInitRenderer();
	rLoadShaders(SHADER_SOURCES);

	struct model *mdl;

	if (! rInitText2D("assets/font.tga")) {
		fatalf("error loading fonts\n");
	}
	if (! (mdl = rOpenMdl("default"))) {
		fatalf("error importing model\n");
	}

	int renderMode = 0;
	double lastFrame = 0;
	glfwSetTime(lastFrame);
	glfwSetWindowUserPointer(win, &renderMode);

	float fov = 45.0f;
	float lx = PI;
	float ly = 0.0f;
	float lspeed = 0.05f; // Look speed
	float mspeed = 3.0f; // Move speed
	vec3 pos = (vec3){0, 0, 5}; // Camera position

	struct light *keyLight = rNewLight();
	keyLight->pos = (vec3){0, 5, 1};
	keyLight->visibility = 2;

	struct camera *cam = rNewCamera(pos, WIDTH, HEIGHT, fov);
	struct network *net = nNewCommandInterface(CMD_PORT);

	if (! net) {
		fatalf("error creating command interface: %s\n", strerror(errno));
	}

	while (! glfwWindowShouldClose(win)) {
		// TODO(cloudhead): Frustum culling (via bounding boxes)
		// TODO(cloudhead): Occlusion culling

		double t = glfwGetTime();
		double ft = (t - lastFrame) * 1000.0f;

		rClear();
		rDrawMdl(mdl);
		rDrawLight(keyLight);
		rDrawFrameTime(ft);
		glfwSwapBuffers(win);

		{
			struct command cmd;

			while (nPollCommand(net, &cmd) != -1) {
				for (int i = 0; i < cmd.argc; i++) {
					printf("<%s> ", cmd.argv[i]);
				}
				printf("\n");
			}
		}
		nPollEvents(NULL);

		if (glfwGetWindowAttrib(win, GLFW_FOCUSED)) {
			glfwPollEvents();
		} else {
			glfwWaitEvents();
		}

		if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(win, GL_TRUE);
		}

		{ // Compute mouse movement
			int width, height;
			double mx, my;
			float centerx, centery;

			glfwGetWindowSize(win, &width, &height);

			centerx = width/2.0f;
			centery = height/2.0f;

			glfwGetCursorPos(win, &mx, &my);
			glfwSetCursorPos(win, centerx, centery);

			double t = glfwGetTime();
			double delta = t - lastFrame;
			lastFrame = t;

			if (glfwGetKey(win, GLFW_KEY_L) != GLFW_PRESS) {
				// How far has the cursor moved from the center of the screen.
				lx += lspeed * delta * (centerx - mx);
				ly += lspeed * delta * (centery - my);
			}

			vec3 dir = (vec3){ // Direction camera is facing
				cosf(ly) * sinf(lx),
				sinf(ly),
				cosf(ly) * cosf(lx)
			};
			vec3 right = (vec3){ // Direction right from camera
				sinf(lx - PI/2),
				0,
				cosf(lx - PI/2)
			};
			vec3 up = vec3cross(right, dir);

			rCameraLookAt(cam, dir, up);

			if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) {
				rCameraMove(cam, vec3scale(dir, delta * mspeed));
			}
			if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) {
				rCameraMove(cam, vec3scale(dir, -delta * mspeed));
			}
			if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) {
				rCameraMove(cam, vec3scale(right, delta * mspeed));
			}
			if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) {
				rCameraMove(cam, vec3scale(right, -delta * mspeed));
			}

			// Setup shaders
			//
			// TODO(cloudhead): Share `proj` and `view` uniforms amongst shaders.
			//
			for (struct shaderSource *src = SHADER_SOURCES; src->name != NULL; src++) {
				struct shader *s = rGetShader(src->name);

				// Skip 'text' shader for now
				if (! strcmp(s->name, "text"))
					continue;

				rUseShader(s);
					if (glfwGetKey(win, GLFW_KEY_L) == GLFW_PRESS) {
						mat4 ndc2world = mat4invert(mat4mul(cam->proj, cam->view));
						vec3 ndc = (vec3){ // Normalized device coordinates (-x)
							-mx / (float)width  * 2.0f + 1.0f,
							+my / (float)height * 2.0f - 1.0f, 0.0f
						};
						vec3 world    = vec3transform(ndc, ndc2world);
						vec3 zero     = vec3transform((vec3){0, 0, 0}, ndc2world);
						vec3 delta    = vec3sub(zero, world);
						keyLight->pos = vec3add(keyLight->pos, delta);
					}
					rSetUniform1i(s, "renderMode", renderMode);
					rSetUniformMatrix4fv(s, "proj", &cam->proj);
					rSetUniformMatrix4fv(s, "view", &cam->view);
					rSetUniform3fv(s, "lightPos", &keyLight->pos);
				rUseShader(0);
			}
		}
	}
	rFreeMdl(mdl);
	rUnloadShaders(SHADER_SOURCES);
	glfwTerminate();

	return 0;
}
