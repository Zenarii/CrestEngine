internal matrix
CrestMatrixPerspective(r32 Theta, r32 Ratio, r32 Near, r32 Far) {
    matrix Result = {0};

    r32 HalfWidth = tan(Theta*0.5f) * Near;
    r32 HalfHeight = HalfWidth / (Ratio);

    Result.Row1 = v4(Near / HalfWidth , 0.0f, 0.0f, 0.0f);
    Result.Row2 = v4(0.0f, (Near/HalfHeight), 0.0f, 0.0f);
    Result.Row3 = v4(0.0f, 0.0f, -(Far+Near)/(Far-Near), -(2*Far*Near)/(Far-Near));
    Result.Row4 = v4(0.0f, 0.0f, -1.0f, 0.0f);
    return Result;
}

internal matrix
LookAt(v3 Target, v3 From) {
    v3 Up = v3(0.f, 1.f, 0.f);

    v3 ReverseDirection = v3(From.x - Target.x, From.y - Target.y, From.z - Target.z);
    ReverseDirection = CrestV3Normalise(ReverseDirection);

    v3 Right = CrestV3Cross(Up, ReverseDirection);
    Right = CrestV3Normalise(Right);

    Up = CrestV3Cross(ReverseDirection, Right);

    matrix Result = {0};
    Result.Row1 = v4(Right.x,            Right.y,            Right.z,            -From.x);
    Result.Row2 = v4(Up.x,               Up.y,               Up.z,               -From.y);
    Result.Row3 = v4(ReverseDirection.x, ReverseDirection.y, ReverseDirection.z, -From.z);
    Result.Row4 = v4(0.f,                0.f,                0.f,                1.f);


    return Result;
}
