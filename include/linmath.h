#include <math.h>
#include <stdio.h>
#include <smmintrin.h>

#define vnorm(A) _Generic((A), vec3: vec3norm, vec4: vec4norm))
#define vdot(A)  _Generic((A), vec3: vec3dot, vec4: vec4dot))

#if defined(SSE)
#define vec4dot(...) ((vec4){ vec4dot_m128((__VA_ARGS__).m128) })
#endif

// TODO: Switch to vec3_t type name, and use
// vec3(x, y, z) as the constructor. This will
// be necessary when using SSE.

typedef union vec2 vec2;
typedef union vec3 vec3;
typedef union vec4 vec4;
typedef struct mat4 mat4;

union vec2 {
	struct {
		float s, t;
	};
	float n[2];
};

union vec3 {
	struct {
		float x, y, z;
	};
	float n[3];
};

union vec4 {
	struct {
		float x, y, z, w;
	};
	float n[4];
	__m128 m128;
};

struct mat4 {
	union {
		vec4 cols[4];
		struct {
			float a1, a2, a3, a4;
			float b1, b2, b3, b4;
			float c1, c2, c3, c4;
			float d1, d2, d3, d4;
		};
	};
};

static const float PI = 3.14149265359f;

static inline vec4 vec4new(float x, float y, float z, float w)
{
	return (vec4){x, y, z, w};
}

static inline vec3 vec3add(vec3 a, vec3 b)
{
	return (vec3){
		a.x + b.x,
		a.y + b.y,
		a.z + b.z
	};
}

static inline vec3 vec3sub(vec3 a, vec3 b)
{
	return (vec3){
		a.x - b.x,
		a.y - b.y,
		a.z - b.z
	};
}

static inline vec3 vec3scale(vec3 a, float s)
{
	return (vec3){
		a.x * s,
		a.y * s,
		a.z * s
	};
}

static inline vec3 vec3rotateY(vec3 a, float angle)
{
	return (vec3){
		 a.x * cosf(angle) + a.z * sinf(angle),
		 a.y,
		-a.x * sinf(angle) + a.z * cosf(angle)
	};
}

static inline float vec3dot(vec3 a, vec3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline vec4 mat4row(mat4 m, int i)
{
	return (vec4){
		{
			m.cols[0].n[i],
			m.cols[1].n[i],
			m.cols[2].n[i],
			m.cols[3].n[i]
		}
	};
}

static inline vec3 vec3cross(vec3 a, vec3 b)
{
	return (vec3){
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}

static inline float vec3len(vec3 v)
{
	return sqrtf(vec3dot(v, v));
}

static inline vec3 vec3norm(vec3 v)
{
	float k = 1.0f / vec3len(v);
	return vec3scale(v, k);
}

static inline vec4 vec4add(vec4 a, vec4 b)
{
	return (vec4){
		a.x + b.x,
		a.y + b.y,
		a.z + b.z,
		a.w + b.w
	};
}

static inline vec4 vec4scale(vec4 v, float s)
{
	return (vec4){
		v.x * s,
		v.y * s,
		v.z * s,
		v.w * s
	};
}

static inline float vec4dot(vec4 a, vec4 b)
{
	return b.x * a.x + b.y * a.y + b.z * a.z + b.w * a.w;
}

static inline vec3 vec3transform(vec3 a, mat4 m)
{
	return (vec3){
		vec4dot(mat4row(m, 0), (vec4){a.x, a.y, a.z, 1.0f}),
		vec4dot(mat4row(m, 1), (vec4){a.x, a.y, a.z, 1.0f}),
		vec4dot(mat4row(m, 2), (vec4){a.x, a.y, a.z, 1.0f})
	};
}

static inline void mat4print(mat4 m, FILE *fp)
{
	fprintf(fp, "%.3f %.3f %.3f %.3f\n", m.cols[0].x, m.cols[1].x, m.cols[2].x, m.cols[3].x);
	fprintf(fp, "%.3f %.3f %.3f %.3f\n", m.cols[0].y, m.cols[1].y, m.cols[2].y, m.cols[3].y);
	fprintf(fp, "%.3f %.3f %.3f %.3f\n", m.cols[0].z, m.cols[1].z, m.cols[2].z, m.cols[3].z);
	fprintf(fp, "%.3f %.3f %.3f %.3f\n", m.cols[0].w, m.cols[1].w, m.cols[2].w, m.cols[3].w);
}

static inline mat4 mat4identity() {
	static mat4 identity = (mat4){
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return identity;
}

static inline mat4 mat4add(mat4 a, mat4 b)
{
	mat4 out;

	for (int i = 0; i < 4; i++) {
		out.cols[i].x = a.cols[i].x + b.cols[i].x;
		out.cols[i].y = a.cols[i].y + b.cols[i].y;
		out.cols[i].z = a.cols[i].z + b.cols[i].z;
		out.cols[i].w = a.cols[i].w + b.cols[i].w;
	}
	return out;
}

static inline mat4 mat4scale(mat4 a, float k)
{
	mat4 out = a;

	out.cols[0].x *= k;
	out.cols[1].y *= k;
	out.cols[2].z *= k;

	return out;
}

static inline mat4 mat4mul(mat4 a, mat4 b)
{
	mat4 out;

	for (int c = 0; c < 4; ++c) {
		for (int r = 0; r < 4; ++r) {
			out.cols[c].n[r] = 0.0f;
			for (int k = 0; k < 4; ++k) {
				out.cols[c].n[r] += a.cols[k].n[r] * b.cols[c].n[k];
			}
		}
	}
	return out;
}

static inline mat4 mat4translate(mat4 a, vec3 t)
{
	mat4 out = a;
	out.cols[3].x = t.x;
	out.cols[3].y = t.y;
	out.cols[3].z = t.z;
	return out;
}

static inline mat4 mat4rotateX(mat4 M, float angle)
{
	float s = sinf(angle);
	float c = cosf(angle);
	mat4 R = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f,    c,    s, 0.0f,
		0.0f,   -s,    c, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return mat4mul(M, R);
}

static inline mat4 mat4rotateY(mat4 M, float angle)
{
	float s = sinf(angle);
	float c = cosf(angle);
	mat4 R = {
		   c, 0.0f,    s, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		  -s, 0.0f,    c, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return mat4mul(M, R);
}

static inline mat4 mat4rotateZ(mat4 M, float angle)
{
	float s = sinf(angle);
	float c = cosf(angle);
	mat4 R = {
		   c,    s, 0.0f, 0.0f,
		  -s,    c, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return mat4mul(M, R);
}

static inline mat4 mat4invert(mat4 M)
{
	mat4 T;
	float s[6];
	float c[6];

	s[0] = M.cols[0].x * M.cols[1].y - M.cols[1].x * M.cols[0].y;
	s[1] = M.cols[0].x * M.cols[1].z - M.cols[1].x * M.cols[0].z;
	s[2] = M.cols[0].x * M.cols[1].w - M.cols[1].x * M.cols[0].w;
	s[3] = M.cols[0].y * M.cols[1].z - M.cols[1].y * M.cols[0].z;
	s[4] = M.cols[0].y * M.cols[1].w - M.cols[1].y * M.cols[0].w;
	s[5] = M.cols[0].z * M.cols[1].w - M.cols[1].z * M.cols[0].w;

	c[0] = M.cols[2].x * M.cols[3].y - M.cols[3].x * M.cols[2].y;
	c[1] = M.cols[2].x * M.cols[3].z - M.cols[3].x * M.cols[2].z;
	c[2] = M.cols[2].x * M.cols[3].w - M.cols[3].x * M.cols[2].w;
	c[3] = M.cols[2].y * M.cols[3].z - M.cols[3].y * M.cols[2].z;
	c[4] = M.cols[2].y * M.cols[3].w - M.cols[3].y * M.cols[2].w;
	c[5] = M.cols[2].z * M.cols[3].w - M.cols[3].z * M.cols[2].w;

	// Assumes it is invertible
	float idet = 1.0f /
		(s[0] * c[5] - s[1] * c[4] + s[2] * c[3] +
		 s[3] * c[2] - s[4] * c[1] + s[5] * c[0]);

	T.cols[0].x = ( M.cols[1].y * c[5] - M.cols[1].z * c[4] + M.cols[1].w * c[3]) * idet;
	T.cols[0].y = (-M.cols[0].y * c[5] + M.cols[0].z * c[4] - M.cols[0].w * c[3]) * idet;
	T.cols[0].z = ( M.cols[3].y * s[5] - M.cols[3].z * s[4] + M.cols[3].w * s[3]) * idet;
	T.cols[0].w = (-M.cols[2].y * s[5] + M.cols[2].z * s[4] - M.cols[2].w * s[3]) * idet;
	T.cols[1].x = (-M.cols[1].x * c[5] + M.cols[1].z * c[2] - M.cols[1].w * c[1]) * idet;
	T.cols[1].y = ( M.cols[0].x * c[5] - M.cols[0].z * c[2] + M.cols[0].w * c[1]) * idet;
	T.cols[1].z = (-M.cols[3].x * s[5] + M.cols[3].z * s[2] - M.cols[3].w * s[1]) * idet;
	T.cols[1].w = ( M.cols[2].x * s[5] - M.cols[2].z * s[2] + M.cols[2].w * s[1]) * idet;
	T.cols[2].x = ( M.cols[1].x * c[4] - M.cols[1].y * c[2] + M.cols[1].w * c[0]) * idet;
	T.cols[2].y = (-M.cols[0].x * c[4] + M.cols[0].y * c[2] - M.cols[0].w * c[0]) * idet;
	T.cols[2].z = ( M.cols[3].x * s[4] - M.cols[3].y * s[2] + M.cols[3].w * s[0]) * idet;
	T.cols[2].w = (-M.cols[2].x * s[4] + M.cols[2].y * s[2] - M.cols[2].w * s[0]) * idet;
	T.cols[3].x = (-M.cols[1].x * c[3] + M.cols[1].y * c[1] - M.cols[1].z * c[0]) * idet;
	T.cols[3].y = ( M.cols[0].x * c[3] - M.cols[0].y * c[1] + M.cols[0].z * c[0]) * idet;
	T.cols[3].z = (-M.cols[3].x * s[3] + M.cols[3].y * s[1] - M.cols[3].z * s[0]) * idet;
	T.cols[3].w = ( M.cols[2].x * s[3] - M.cols[2].y * s[1] + M.cols[2].z * s[0]) * idet;

	return T;
}

static inline mat4 mat4perspective(float fov, float aspect, float n, float f)
{
	mat4 out;

	float const s = 1.0f / tan(fov * 0.5f);

	out.cols[0].x = s / aspect;
	out.cols[0].y = 0.0f;
	out.cols[0].z = 0.0f;
	out.cols[0].w = 0.0f;

	out.cols[1].x = 0.0f;
	out.cols[1].y = s;
	out.cols[1].z = 0.0f;
	out.cols[1].w = 0.0f;

	out.cols[2].x = 0.0f;
	out.cols[2].y = 0.0f;
	out.cols[2].z = (f + n) / (n - f);
	out.cols[2].w = -1.0f;

	out.cols[3].x = 0.0f;
	out.cols[3].y = 0.0f;
	out.cols[3].z = (2.0f * f * n) / (n - f);
	out.cols[3].w = 0.0f;

	return out;
}

static inline mat4 mat4lookAt(vec3 eye, vec3 center, vec3 up)
{
	mat4 out;

	vec3 f = vec3norm(vec3sub(center, eye));
	vec3 s = vec3norm(vec3cross(f, up));
	vec3 t = vec3cross(s, f);

	out.cols[0].x =  s.x;
	out.cols[0].y =  t.x;
	out.cols[0].z = -f.x;
	out.cols[0].w =  0.f;

	out.cols[1].x =  s.y;
	out.cols[1].y =  t.y;
	out.cols[1].z = -f.y;
	out.cols[1].w =  0.f;

	out.cols[2].x =  s.z;
	out.cols[2].y =  t.z;
	out.cols[2].z = -f.z;
	out.cols[2].w =  0.f;

	out.cols[3].x = -vec3dot(s, eye);
	out.cols[3].y = -vec3dot(t, eye);
	out.cols[3].z =  vec3dot(f, eye);
	out.cols[3].w =  1.f;

	return out;
}

