//TODO(Zen): Maybe add asymptotic movement/zooming to add smoothness

#define CAMERA_OUT_SPEED 10.f
#define CAMERA_IN_SPEED 4.5f
#define CAMERA_MAX_DISTANCE 20.f
#define CAMERA_MIN_DISTANCE 7.5f
#define CAMERA_ZOOM_SPEED 8.f
#define CAMERA_MAX_ANGLE (0.45f * PI)
#define CAMERA_MIN_ANGLE (0.1f * PI)

typedef struct camera camera;
struct camera {
    r32 Zoom;
    r32 Speed;
    //camera
    r32 Rotation;
    v3 Translation;
    //around the point it's looking at
    r32 Swivel;
    v3 Position;
};
