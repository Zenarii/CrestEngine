typedef struct matrix4x4 {
    union {
        struct {
            v4 Row1;
            v4 Row2;
            v4 Row3;
            v4 Row4;
        };

        r32 Elements[16];
    };
} matrix;

//Note(Zen): Create with columns
internal matrix
CrestMatrixInitR(v4 Row1, v4 Row2, v4 Row3, v4 Row4) {
    matrix Result = {
        Row1,
        Row2,
        Row3,
        Row4
    };
    return Result;
}
#define MatrixR(R1, R2, R3, R4) CrestMatrixInitR(R1, R2, R3, R4)

internal int
matrix4index(int n, int m) {
  return 4*n + m;
}

//Note(Zen): Create Matrix with Rows
internal matrix
CrestMatrixInitC(v4 Column1, v4 Column2, v4 Column3, v4 Column4) {
    matrix Result = {
        v4(Column1.x, Column2.x, Column3.x, Column4.x),
        v4(Column1.y, Column2.y, Column3.y, Column4.y),
        v4(Column1.z, Column2.z, Column3.z, Column4.z),
        v4(Column1.w, Column2.w, Column3.w, Column4.w)
    };
    return Result;
}
#define MatrixC(C1, C2, C3, C4) CrestMatrixInitC(C1, C2, C3, C4)

internal matrix
CrestM4MultM4(matrix m1, matrix m2) {
  matrix result = {0};
  for(int n = 0; n < 4; ++n) {
    for(int m = 0; m < 4; ++m) {
      int index = matrix4index(n, m);
      for(int i = 0; i < 4; ++i) {
        result.Elements[index] += m1.Elements[4*n+i] * m2.Elements[4*i+m];
      }
    }
  }
  return result;
}


enum AXES {
    CREST_AXIS_X,
    CREST_AXIS_Y,
    CREST_AXIS_Z,
};

internal matrix
CrestMatrixRotation(r32 Angle, enum AXES Axis) {
    matrix Result = {0};
    switch (Axis) {
        case CREST_AXIS_X: {
            Result.Row1 = v4(1.f, 0.f, 0.f, 0.f);
            Result.Row2 = v4(0.f, cosf(Angle), -sinf(Angle), 0.f);
            Result.Row3 = v4(0.f, sinf(Angle), cosf(Angle), 0.f);
            Result.Row4 = v4(0.f, 0.f, 0.f, 1.f);
        } break;

        case CREST_AXIS_Y: {
            Result.Row1 = v4(cosf(Angle), 0.f, sinf(Angle), 0.f);
            Result.Row2 = v4(0.f, 1.f, 0.f, 0.f);
            Result.Row3 = v4(-sinf(Angle), 0.f, cosf(Angle), 0.f);
            Result.Row4 = v4(0.f, 0.f, 0.f, 1.f);
        } break;

        case CREST_AXIS_Z: {
            Result.Row1 = v4(cosf(Angle), -sinf(Angle), 0.f, 0.f);
            Result.Row2 = v4(sinf(Angle),  cosf(Angle), 0.f, 0.f);
            Result.Row3 = v4(0.f,          0.f,         1.f, 0.f);
            Result.Row4 = v4(0.f,          0.f,         0.f, 1.f);
        } break;
    }
    return Result;
}

internal matrix
CrestMatrixTranslation(v3 Translation) {
    matrix Result = {0};
    Result.Row1 = v4(1.f, 0.f, 0.f, Translation.x);
    Result.Row2 = v4(0.f, 1.f, 0.f, Translation.y);
    Result.Row3 = v4(0.f, 0.f, 1.f, Translation.z);
    Result.Row4 = v4(0.f, 0.f, 0.f, 1.f);
    return Result;
}

#define TranslateMatrix(Matrix, TranslationVector)
