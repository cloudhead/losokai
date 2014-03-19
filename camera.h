
struct camera {
    vec3 pos;
    vec3 center;
    vec3 eye;
    vec3 up;
    mat4 proj;
    mat4 view;

    float fov;
    float znear;
    float zfar;

    int resx;
    int resy;
};

extern struct camera *rNewCamera(vec3 pos, int width, int height, float fov);
extern void rCameraMove(struct camera *c, vec3 dir);
extern void rCameraLookAt(struct camera *c, vec3 dir, vec3 up);
