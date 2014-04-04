struct vertex {
	vec3  pos;
	vec2  uv;
	vec3  normal;
	vec4  tangent;
	int   bones[4];
	float weights[4];
};
