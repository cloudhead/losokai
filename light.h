struct light {
	int   visibility;
	float brightness;
	vec3  pos;
	vec3  dir;
	vec3  rgb;
};

extern struct light *rNewLight();
extern void rDrawLight(struct light *);
