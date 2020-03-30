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
