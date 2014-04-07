
enum gbufferTextureType {
	GBUFFER_TEXTURE_TYPE_POSITION,
	GBUFFER_TEXTURE_TYPE_DIFFUSE,
	GBUFFER_TEXTURE_TYPE_NORMAL,
	GBUFFER_TEXTURE_TYPE_TEXCOORD,
	GBUFFER_NTEXTURES
};

struct gbuffer {
	int width, height;
	GLuint fbo;
	GLuint textures[GBUFFER_NTEXTURES];
	GLuint depth;
};

extern struct gbuffer *rNewGbuffer(unsigned int, unsigned int, GLint, GLint);
extern void rGbufferRBind(struct gbuffer *);
extern void rGbufferWBind(struct gbuffer *);
extern void rGbufferSetRBuffer(struct gbuffer *g, enum gbufferTextureType);
extern void rGbufferBlit(struct gbuffer *, enum gbufferTextureType, int, int, int, int);
