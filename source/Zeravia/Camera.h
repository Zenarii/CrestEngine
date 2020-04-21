#define CAMERA_SPEED 20.f

typedef struct camera camera;
struct camera {
    //camera
    r32 Rotation;
    v3 Translation;
    //around the point it's looking at
    r32 Swivel;
    v3 Position;
};
