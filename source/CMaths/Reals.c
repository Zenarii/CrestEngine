internal r32
CrestLerp(r32 a, r32 b, r32 t) {
    a += (b - a) * t;
    return a;
}
