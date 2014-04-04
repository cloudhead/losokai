struct material {
	struct texture *textures[TEXTURE_TYPES];
	struct shader  *shader;
	GLuint         sampler;
};

extern struct material *rNewMaterial(struct shader *, const char *, const char *);
extern struct material *rNewBasicMaterial(struct shader *);
extern void rSetMaterialProperty4fv(struct material *, const char *, vec4);
