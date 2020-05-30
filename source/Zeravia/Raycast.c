internal ray_cast
MakeRaycast(camera * Camera, v2 ScreenPoint, matrix View, matrix Projection) {
    v4 RayClip = v4(0, 0, -1.f, 1.f);
    RayClip.x = ScreenPoint.x;
    RayClip.y = ScreenPoint.y;

    v4 RayEye = CrestMatrixMultipyV4(CrestMatrixInverse(Projection), RayClip);
    RayEye.z = -1.f;
    RayEye.w = 0.f;
    v4 RayWorld = CrestMatrixMultipyV4(CrestMatrixInverse(View), RayEye);
    v3 RayDirection = CrestV3Normalise(v3(RayWorld.x, RayWorld.y, RayWorld.z));

    v3 RayOrigin = GetCameraLocation(Camera);

    ray_cast RayCast = {RayOrigin, RayDirection};
    return RayCast;
}

internal ray_cast
MakeRaycastFromMouse(camera * Camera, app * App, matrix View, matrix Projection) {
    v2 Mouse = {0};

    Mouse.x = (2.f * App->Mouse.Position.x) / App->ScreenWidth - 1.f;
    Mouse.y = 1.f - (2.f * App->Mouse.Position.y) / App->ScreenHeight;

    return MakeRaycast(Camera, Mouse, View, Projection);
}
//Note(Zen): Only works when first setting a variable
#define InlineV3Sub(v1, v2) {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z}
#define InlineV3Cross(a, b) {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}
#define InlineV3Dot(a, b) (a.x * b.x + a.y * b.y + a.z * b.z)

internal collision_result
RayTriangleIntersect(v3 RayOrigin, v3 RayDirection, collision_triangle * Triangle) {
    collision_result Result = {0};

    const r32 EPSILON = 0.000001;
    v3 V0 = Triangle->Vertex0;
    v3 V1 = Triangle->Vertex1;
    v3 V2 = Triangle->Vertex2;

    r32 a, f, u, v;
    v3 Edge1 = InlineV3Sub(V1, V0);
    v3 Edge2 = InlineV3Sub(V2, V0);

    v3 h = InlineV3Cross(RayDirection, Edge2);
    a = InlineV3Dot(Edge1, h);
    //Note(Zen): Means the triangle is parallel to the ray
    if(a > -EPSILON && a < EPSILON) return Result;

    f = 1.f/a;
    v3 s = InlineV3Sub(RayOrigin, V0);
    u = f * InlineV3Dot(s, h);

    if(u < 0.f || u > 1.f) return Result;

    v3 q = InlineV3Cross(s, Edge1);
    v = f * InlineV3Dot(RayDirection, q);

    if(v < 0.f || (u + v) > 1.f) return Result;

    r32 t = f * InlineV3Dot(Edge2, q);
    if (t > EPSILON) {
        Result.DidIntersect = 1;
        v3 RayLength = v3(RayDirection.x * t, RayDirection.y * t, RayDirection.z * t);
        Result.IntersectionPoint = CrestV3Add(RayOrigin, RayLength);
        return Result;
    }
    //Note(Zen): Line intersects but ray does not
    else return Result;
}

//WARNING(Zen): This function returns a pointer to a static array
// So calls will overwrite the return everywhere
internal collision_result_set
RayMeshAllIntersects(ray_cast Ray, collision_mesh Mesh) {
    //stub
}

internal b32
RayMeshDoesIntersect(ray_cast Ray, collision_mesh * Mesh) {
    for(i32 i = 0; i < Mesh->TriangleCount; ++i) {
        collision_result Hit = RayTriangleIntersect(Ray.Origin, Ray.Direction, &Mesh->Triangles[i]);
        if(Hit.DidIntersect) {
            return 1;
        }
    }
    return 0;
}

internal b32
RayMeshDoesIntersectL(ray_cast Ray, large_collision_mesh * Mesh) {
    for(i32 i = 0; i < 10; ++i) {
        collision_result Hit = RayTriangleIntersect(Ray.Origin, Ray.Direction, &Mesh->Triangles[i]);
        if(Hit.DidIntersect) {
            return 1;
        }
    }
    return 0;
}
