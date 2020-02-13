#include "../CrestMaths.cpp"

typedef struct Camera {
    vector3 Position;
    vector3 Target;
    vector3 Direction;
    vector3 Right;
    vector3 Up;
} Camera;

//Note(Zen): If input seems "flipped" check this function
internal matrix4
LookAt(vector3 Position, vector3 Target, vector3 Up) {
    vector3 Right = CrestV3Normalise(CrestV3Cross(Target, Up));
    matrix4 m1 = {};
    m1.Row1 = CrestV4FromV3(Right, 0.0f);
    m1.Row2 = CrestV4FromV3(Up, 0.0f);
    m1.Row3 = CrestV4FromV3(CrestV3Sub(Position, Target), 0.0f);
    m1.Row4 = {0.0f, 0.0f, 0.0f, 1.0f};
    matrix4 result = CrestM4MultM4(m1, CrestTranslationMatrix(Position.x, Position.y, Position.z));
    return result;
}

internal void
doCamera(platform * Platform) {

}

internal Camera
initCamera() {
    Camera result = {};
    result.Position = Vector3(0.0f, 0.0f, -0.3f);
    result.Target = Vector3(0.0f, 0.0f, 0.0f);
    result.Direction = CrestV3Normalise(CrestV3Sub(result.Position, result.Target));
    vector3 Up = Vector3(0.0f, 1.0f, 0.0f);
    result.Right = CrestV3Normalise(CrestV3Cross(Up, result.Direction));
    //Note(Zen): No need to normalise as both crossed vectors are normalised already
    result.Up = CrestV3Cross(result.Direction, result.Right);
    return result;
}
