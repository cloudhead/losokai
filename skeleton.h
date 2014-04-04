struct bone {
	char         *name;
	mat4         transform;
	float        length;
	mat4         offset;
	unsigned int parentId;
};

struct skeleton {
	struct bone     *bones;
	size_t          nbones;
};

void rDrawSkeleton(struct skeleton *, mat4 *);
