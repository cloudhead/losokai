struct vertex {
	vec3 pos;
	vec3 normal;
	vec4 tangent;
	vec2 uv;
};

struct mesh {
	char            *name;
	GLuint          vbo;
	GLuint          vao;
	GLuint          ebo;
	struct vertex   *vertices;
	size_t          nvertices;
	unsigned int    *faces;
	size_t          nfaces;
	struct material *material;
	bool            isVisible;
};

extern void meshInit(struct mesh *);
extern void meshFree(struct mesh *);
extern void rMeshDraw(struct mesh *, mat4 *);
extern struct mesh *rNewMesh(const char *);
