#define CAMERA_OUT_SPEED 25.f
#define CAMERA_IN_SPEED 8.f
#define CAMERA_MAX_DISTANCE 22.f
#define CAMERA_MIN_DISTANCE 8.f
#define CAMERA_ZOOM_SPEED 13.f
#define CAMERA_ZOOM_INTERPOLATION 0.2f
#define CAMERA_MAX_ANGLE (0.45f * PI)
#define CAMERA_MIN_ANGLE (0.1f * PI)

typedef struct camera camera;
struct camera {
    r32 TargetZoom;
    r32 Zoom;
    r32 Speed;
    //camera
    r32 Rotation;
    v3 Translation;
    //around the point it's looking at
    r32 Swivel;
    v3 Position;
    v3 TargetPosition;
};
