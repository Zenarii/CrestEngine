#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef int8_t b8;
typedef int16_t b16;
typedef int32_t b32;
typedef int64_t b64;

typedef float r32;
typedef double r64;

typedef uintptr_t pointer;

#define KiloBytes(x) x * 1024
#define MegaBytes(x) x * 1024 * 1024
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define internal static
#define global static

#define PI 3.14159265358f

//Vector types
//~
typedef union v4 {
    struct {
        struct {
            r32 x, y;
        };
        union {
            struct {
                r32 z, w;
            };
            struct {
                r32 width, height;
            };
        };
    };
    float elements[4];
} v4;


internal v4
V4Init(r32 x, r32 y, r32 z, r32 w) {
    v4 v = {x,y,z,w};
    return v;
}

#define v4(x, y, z, w) V4Init(x, y, z, w)

typedef union v4i {
    struct {
        struct {
            i32 x, y;
        };
        union {
            struct {
                i32 z, w;
            };
            struct {
                i32 width, height;
            };
        };
    };
    i32 elements[4];
} v4i;

typedef union v2 {
    struct {
        r32 x, y;
    };
    struct {
        r32 u, v;
    };
} v2;

internal v2
V2Init(r32 x, r32 y) {
    v2 v = {x,y};
    return v;
}

#define v2(x, y) V2Init(x, y)

typedef struct v3 {
    r32 x, y, z;
} v3;

internal v3
V3Init(r32 x, r32 y, r32 z) {
    v3 v = {x,y,z};
    return v;
}

#define v3(x, y, z) V3Init(x, y, z)

internal b32
IsPowerOfTwo(i32 x) {
    return (x & (x-1)) == 0;
}
