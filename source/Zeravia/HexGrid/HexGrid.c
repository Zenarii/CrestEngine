
internal hex_cell
CreateCell(int x, int z) {
    hex_cell Result = {0};
    Result.Position = v3((x + z * 0.5f - z/2) * HEX_INNER_DIAMETER, 0.f, z * HEX_OUTER_RADIUS * 1.5f);
    return Result;
}
/*
internal v3
HexCoordsToCartesian(v3 HexCoords) {
    v3 Cartesian = v3();
    return Cartesian;
}
*/
internal hex_coordinates
CartesianToHexCoords(int x, int z) {
    hex_coordinates HexCoords = {x - z/2, 0.f,  z};
    HexCoords.y = -HexCoords.z - HexCoords.x; // x + y + z = 0;
    return HexCoords;
}

internal collision_triangle
CreateTriangle(v3 v0, v3 v1, v3 v2) {
    collision_triangle Result = {v0, v1, v2};
    return Result;
};

//Note(Zen):
//Using the moller-trumbore intersection algorithm
//https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
typedef struct collision_result collision_result;
struct collision_result {
    b32 DidIntersect;
    v3 IntersectionPoint;
};

internal collision_result
RayTriangleIntersect(v3 RayOrigin, v3 RayDirection, collision_triangle * Triangle) {
    collision_result Result = {0};

    const r32 EPSILON = 0.000001;
    v3 V0 = Triangle->Vertex0;
    v3 V1 = Triangle->Vertex1;
    v3 V2 = Triangle->Vertex2;

    v3 Edge1, Edge2, h, s, q; //Yeah idk what these are meant to correspond to
    r32 a, f, u, v;
    Edge1 = CrestV3Sub(V1, V0);
    Edge2= CrestV3Sub(V2, V0);

    h = CrestV3Cross(RayDirection, Edge2);
    a = CrestV3Dot(Edge1, h);
    //Note(Zen): Means the triangle is parallel to the ray
    if(a > -EPSILON && a < EPSILON) return Result;

    f = 1.f/a;
    s = CrestV3Sub(RayOrigin, V0);
    u = f * CrestV3Dot(s, h);

    if(u < 0.f || u > 1.f) return Result;

    q = CrestV3Cross(s, Edge1);
    v = f * CrestV3Dot(RayDirection, q);

    if(v < 0.f || (u + v) > 1.f) return Result;

    r32 t = f * CrestV3Dot(Edge2, q);
    if (t > EPSILON) {
        Result.DidIntersect = 1;
        v3 RayLength = v3(RayDirection.x * t, RayDirection.y * t, RayDirection.z * t);
        Result.IntersectionPoint = CrestV3Add(RayOrigin, RayLength);
        return Result;
    }
    //Note(Zen): Line intersects but ray does not
    else return Result;
}

/*
internal void
HexMeshAddTri() {
    Assert() Indices < MAX_INDICES and Vertices < MAX_VERTICES for the chunk
}
*/
