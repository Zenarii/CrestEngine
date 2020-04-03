internal v3
CrestV3Cross(v3 a, v3 b) {
    v3 Result = {0};
    Result.x = a.y * b.z - a.z * b.y;
    Result.y = a.z * b.x - a.x * b.z;
    Result.z = a.x * b.y - a.y * b.x;
    return Result;
}

internal v3
CrestV3Normalise(v3 v) {
    r32 MagnitudeSquared = v.x * v.x + v.y * v.y + v.z * v.z;

    if(MagnitudeSquared != 1.f) {
        r32 Magnitude = sqrt(MagnitudeSquared);
        v = v3(v.x/Magnitude, v.y/Magnitude, v.z/Magnitude);
    }
    return v;
}

internal v3
CrestV3Add(v3 a, v3 b) {
    v3 Result = v3(a.x + b.x, a.y + b.y, a.z + b.z);
    return Result;
}

internal v3
CrestV3Sub(v3 a, v3 b) {
    v3 Result = v3(a.x - b.x, a.y - b.y, a.z - b.z);
    return Result;
}

internal r32
CrestV3Dot(v3 a, v3 b) {
    r32 Result = a.x * b.x + a.y * b.y + a.z * b.z;
    return Result;
}
internal r32
CrestV4Dot(v4 a, v4 b) {
    r32 Result = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    return Result;
}

internal v3
CrestV3Scale(v3 v, r32 s) {
    v3 Result = v3(v.x * s, v.y * s, v.z * s);
    return Result;
}
