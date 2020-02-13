typedef union matrix4 {
    struct {
        vector4 Row1;
        vector4 Row2;
        vector4 Row3;
        vector4 Row4;
    };
    real32 elements[16];
} matrix4;

internal int
matrix4index(int n, int m) {
  return 4*n + m;
}

//Note(Zen): initialise as diagonal matrix
internal matrix4
CrestMatrix4InitF(real32 value) {
    matrix4 Result = {};
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
    /*for(int i = 0; i < 4; ++i) {
    for(int j = 0; j < 4; ++j) {
    int index = matrix4index(i, j);
    result.elements[i] += m.elements[index] * v.elements[j];
    }
    }*/
    result.x = CrestV4Dot(m.Row1, v);
    result.y = CrestV4Dot(m.Row2, v);
    result.z = CrestV4Dot(m.Row3, v);
    result.w = CrestV4Dot(m.Row4, v);

    return result;
}
