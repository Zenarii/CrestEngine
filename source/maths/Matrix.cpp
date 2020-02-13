//Note(Zen): This matrix is row-major
//so need to set transpose true for OpenGL functions
typedef union matrix4 {
    struct {
        vector4 Row1;
        vector4 Row2;
        vector4 Row3;
        vector4 Row4;
    };
    real32 First;
    real32 elements[16];
} matrix4;

internal int
matrix4index(int n, int m) {
  return 4*n + m;
}

enum crest_axis {
    CREST_AXIS_X,
    CREST_AXIS_Y,
    CREST_AXIS_Z,
};

//Matrix Creation
//~
//Note(Zen): initialise as diagonal matrix
internal matrix4
CrestMatrix4InitF(real32 value) {
    matrix4 result = {};
    for(int i = 0; i < 4; ++i) {
        int index = matrix4index(i, i);
        result.elements[index] = value;
    }
    return result;
}
#define Matrix4(value) CrestMatrix4InitF(value)

internal matrix4
CrestTranslationMatrix(real32 dx, real32 dy, real32 dz) {
  matrix4 result = Matrix4(1.0f);
  result.elements[3] = dx;
  result.elements[7] = dy;
  result.elements[11] = dz;
  return result;
}

internal matrix4
CrestScaleMatrix(real32 ScaleFactor) {
    matrix4 result = {};
    for(int i = 0; i < 3; ++i) {
        int index = matrix4index(i, i);
        result.elements[index] = ScaleFactor;
    }
    return result;
}

//TODO(Zen): Improve this
internal matrix4
CrestRotationMatrix(real32 radians, crest_axis axis) {
    matrix4 Result = {};
    switch(axis) {
        case CREST_AXIS_X: {
            Result.Row1 = {1, 0, 0, 0};
            Result.Row2 = {0, cosf(radians), -sinf(radians), 0};
            Result.Row3 = {0, sinf(radians),  cosf(radians), 0};
            Result.Row4 = {0, 0, 0, 1};
        } break;

        case CREST_AXIS_Y: {
            Result.Row1 = {cosf(radians), 0, sinf(radians), 0};
            Result.Row2 = {0, 1, 0, 0};
            Result.Row3 = {-sinf(radians), 0, cosf(radians), 0};
            Result.Row4 = {0, 0, 0, 1};
        } break;

        case CREST_AXIS_Z: {
            Result.Row1 = {cosf(radians), -sinf(radians), 0, 0};
            Result.Row2 = {sinf(radians),  cosf(radians), 0, 0};
            Result.Row3 = {0, 0, 1, 0};
            Result.Row4 = {0, 0, 0, 1};
        }
    }
    return Result;
}

//Note(Zen): Adapted from https://www.songho.ca/opengl/gl_projectionmatrix.html
//Ratio is width/height
internal matrix4
CrestProjectionMatrix(real32 Theta, real32 Ratio, real32 Near, real32 Far) {
    matrix4 Result = {};

    real32 HalfWidth = tan(Theta * 0.5f) * Near;
    real32 HalfHeight = HalfWidth / Ratio;

    Result.Row1 = {Near / HalfWidth , 0.0f, 0.0f, 0.0f};
    Result.Row2 = {0.0f, 2.0f*(Near/HalfHeight), 0.0f, 0.0f};
    Result.Row3 = {0.0f, 0.0f, -(Far+Near)/(Far-Near), -(2*Far*Near)/(Far-Near)};
    Result.Row4 = {0.0f, 0.0f, -1.0f, 0.0f};

    return Result;
}

//Matrix operations
//~
internal matrix4
CrestM4MultM4(matrix4 m1, matrix4 m2) {
  matrix4 result = {};
  for(int n = 0; n < 4; ++n) {
    for(int m = 0; m < 4; ++m) {
      int index = matrix4index(n, m);
      for(int i = 0; i < 4; ++i) {
        result.elements[index] += m1.elements[4*n+i]*m2.elements[4*i+m];
      }
    }
  }
  return result;
}

internal matrix4
M4Add(matrix4 m1, matrix4 m2) {
  matrix4 Result = {};
  for(int i = 0; i < 16; ++i) {
    Result.elements[i] = m1.elements[i] + m2.elements[i];
  }
  return Result;
}

internal matrix4
M4Sub(matrix4 m1, matrix4 m2) {
  matrix4 Result = {};
  for(int i = 0; i < 16; ++i) {
    Result.elements[i] = m1.elements[i] - m2.elements[i];
  }
  return Result;
}

internal vector4
CrestM4MultV4(matrix4 m, vector4 v) {
    vector4 result = {};
    result.x = CrestV4Dot(m.Row1, v);
    result.y = CrestV4Dot(m.Row2, v);
    result.z = CrestV4Dot(m.Row3, v);
    result.w = CrestV4Dot(m.Row4, v);

    return result;
}
