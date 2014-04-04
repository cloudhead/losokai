typedef struct dict *dict_t;

struct shader {
	GLuint handle;
	dict_t uniforms;
	char   name[];
};

struct shaderSource {
	char *name, *vert, *frag;
};

extern struct shader *rGetShader(const char *);
extern struct shader *rNewShader(const char *, GLenum);
extern void rDeleteShader(struct shader *);
extern bool rLoadShaders(struct shaderSource *);
extern void rUnloadShaders(struct shaderSource *);
extern void rUseShader(struct shader *);
extern void rSetUniformMatrix4fv(struct shader *, const char *, mat4 *);
extern void rSetUniform3fv(struct shader *, const char *, vec3 *);
extern void rSetUniform4fv(struct shader *, const char *, vec4 *);
extern void rSetUniform1i(struct shader *, const char *, GLint);
