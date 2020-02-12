//Vector3
//~
typedef union vector3 {
    struct {
        real32 x, y, z;
    };
    struct {
        real32 r, g, b;
    };
    real32 elements[3];
} vector3, colour3;

vector3 CrestV3Init(real32 x, real32 y, real32 z) {
    vector3 v = {x, y, z};
    return v;
};
#define vector3(x, y, z) CrestV3Init(x, y, z)
#define colour3(x, y, z) CrestV3Init(x, y, z)

vector3 CrestV3Add(vector3 v1, vector3 v2) {
    vector3 v = {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
    return v;
}

//TODO(Zen): Make this better (warning C4172: returning address of local variable or temporary: buffer)
/*
char * CrestV3ToString(vector3 v) {
    char buffer[256];
    snprintf(buffer, 64, "(%f, %f, %f)\n", v.x, v.y, v.z);
    return buffer;
}
*/

//Vector4
//~
typedef union vector4 {
    struct {
        real32 x, y, z, w;
    };
    struct {
        real32 r, g, b, a;
    };
    real32 elements[4];
} vector4, colour4;

vector4 CrestV4Init(real32 x, real32 y, real32 z, real32 w) {
    vector4 v = {x, y, z, w};
    return v;
};

#define vector4(x, y, z, w) CrestV4Init(x, y, z, w)
#define colour4(x, y, z, w) CrestV4Init(x, y, z, w)
