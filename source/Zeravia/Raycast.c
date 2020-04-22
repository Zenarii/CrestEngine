internal ray_cast
MakeRaycast(camera * Camera, app * App, matrix View, matrix Projection) {
    v4 RayClip = v4(0, 0, -1.f, 1.f);
    RayClip.x = (2.f * App->Mouse.Position.x) / App->ScreenWidth - 1.f;
    RayClip.y = 1.f - (2.f * App->Mouse.Position.y) / App->ScreenHeight;

    v4 RayEye = CrestMatrixMultipyV4(CrestMatrixInverse(Projection), RayClip);
    RayEye.z = -1.f;
    RayEye.w = 0.f;
    v4 RayWorld = CrestMatrixMultipyV4(CrestMatrixInverse(View), RayEye);
    v3 RayDirection = CrestV3Normalise(v3(RayWorld.x, RayWorld.y, RayWorld.z));

    v3 RayOrigin = GetCameraLocation(Camera);

    ray_cast RayCast = {RayOrigin, RayDirection};
    return RayCast;
}
