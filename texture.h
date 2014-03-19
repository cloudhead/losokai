enum textureType {
	TEXTURE_TYPE_DIFFUSE,
	TEXTURE_TYPE_NORMAL,
	TEXTURE_TYPE_SPECULAR,
	TEXTURE_TYPES
};

struct texture {
	GLuint handle;
	GLuint sampler;
	GLint  uniform;
	int    index;
};

GLuint rNewSampler(GLuint minFilter, GLuint magFilter);
struct texture *rNewTexture(void *pixels, int w, int h, GLint format);
struct texture *rTextureFromPath(const char *path, GLint format);

void rGenerateMipmap(struct texture *t);

const char *rTextureExtension(enum textureType t);
GLint rTextureFormat(enum textureType t);
