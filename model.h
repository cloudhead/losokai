
struct model {
	const char   *name;
	struct mesh **meshes;
	size_t       nmeshes;
};

extern struct model *rOpenMdl(const char *);
extern void rDrawMdl(struct model *);
extern void rFreeMdl(struct model *);
extern bool rUseMdlShader(struct model *, GLuint);
