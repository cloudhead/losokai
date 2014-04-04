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
	struct skeleton *skeleton;
	bool            isVisible;
};

extern void meshInit(struct mesh *);
extern void meshFree(struct mesh *);
extern void rDrawMesh(struct mesh *, mat4 *);
extern struct mesh *rNewMesh(const char *, struct material *, size_t, struct vertex *, size_t, unsigned int *, struct skeleton *);
