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
#define Vector3(x, y, z) CrestV3Init(x, y, z)
#define Colour3(x, y, z) CrestV3Init(x, y, z)

vector3 CrestV3Add(vector3 v1, vector3 v2) {
    vector3 v = {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
    return v;
}

vector3 CrestV3Sub(vector3 v1, vector3 v2) {
    vector3 v = {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
    return v;
}

vector3 CrestV3DivideF(vector3 v, real32 f) {
    vector3 result = {v.x/f, v.y/f, v.z/f};
    return result;
}

real32 CrestV3Magnitude(vector3 v) {
    return sqrt((v.x*v.x)+(v.y*v.y)+(v.z*v.z));
}

vector3 CrestV3Normalise(vector3 v) {
    return CrestV3DivideF(v, CrestV3Magnitude(v));
}

//Note(Zen): https://en.wikipedia.org/wiki/Cross_product#Coordinate_notation
vector3 CrestV3Cross(vector3 a, vector3 b) {
    vector3 result = {};
    result.x = a.y*b.z - a.z*b.y;
    result.y = a.z*b.x - a.x*b.z;
    result.z = a.x*b.y - a.y*b.x;
    return result;
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

internal vector4
CrestV4Init(real32 x, real32 y, real32 z, real32 w) {
    vector4 v = {x, y, z, w};
    return v;
};

internal vector4
CrestV4FromV3(vector3 v, real32 w) {
    return {v.x, v.y, v.z, w};
}

#define Vector4(x, y, z, w) CrestV4Init(x, y, z, w)
#define Colour4(x, y, z, w) CrestV4Init(x, y, z, w)

vector4 CrestV4Add(vector4 v1, vector4 v2) {
    vector4 v = {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w};
    return v;
}

real32 CrestV4Dot(vector4 v1, vector4 v2) {
    real32 result = (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z) + (v1.w * v2.w);
    return result;
}

vector4 CrestV4Sub(vector4 v1, vector4 v2) {
    vector4 result = {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w};
    return result;
}

//Vector2
//~
typedef union {
    struct {
        real32 x, y;
    };
} vector2;
