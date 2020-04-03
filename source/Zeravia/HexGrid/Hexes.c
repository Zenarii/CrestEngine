
internal hex_cell
CreateCell(int x, int z) {
    hex_cell Result = {0};
    Result.Position = v3((x + z * 0.5f - z/2) * HEX_INNER_DIAMETER, 0.f, z * HEX_OUTER_RADIUS * 1.5f);
    Result.Colour = v3(1.f, 1.f, 1.f);
    return Result;
}

//Note(Zen): For some reason C doesn't have a round function apparently?
internal i32
RoundFromFloat(r32 in) {
    return (in >= 0) ? (in + 0.5f) : (in - 0.5f);
}

//TODO(Zen): Make this more accurate
internal hex_coordinates
CartesianToHexCoords(r32 x, r32 z) {
    r32 x2 = x / (HEX_INNER_RADIUS * 2.f);
    r32 y2 = -x2;
    r32 offset = z / (HEX_OUTER_RADIUS * 3.f);
    x2 -= offset;
    y2 -= offset;

    i32 IntX = RoundFromFloat(x2);
    i32 IntY = RoundFromFloat(y2);
    i32 IntZ = RoundFromFloat(-x2 - y2);

    if(IntX + IntY + IntZ != 0) {
        r32 ErrorX = abs(x2 - IntX);
        r32 ErrorY = abs(y2 - IntY);
        r32 ErrorZ = abs(-x2 -y2 - IntZ);

        //Note(Zen): if the largest error in X remake x
        if(ErrorX > ErrorY && ErrorX > ErrorZ) {
            IntX = -IntY - IntZ;
        }
        else if(ErrorZ > ErrorY) {
            IntZ = -IntY - IntX;
        }
    }


    hex_coordinates HexCoords = {IntX, -IntX -IntZ, IntZ};
    return HexCoords;
}

internal i32
GetCellIndex(hex_coordinates Coords) {
    u32 Result = Coords.z * HEX_CHUNK_WIDTH + Coords.x + (Coords.z/2);
    if(Result > HEX_CHUNK_WIDTH * HEX_CHUNK_HEIGHT) return -1;

    return Result;
}


/*
Collisions Calculations:
*/

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
    Drawing the Mesh to the screen
*/

internal void
DrawHexMesh(C3DRenderer * Renderer, hex_mesh * Mesh) {
    glUseProgram(Renderer->Shader);
    //Note(Zen): Set Active Textures
    for(i32 i = 0; i < Renderer->ActiveTextures; ++i) {
        glBindTextureUnit(i, Renderer->Textures[i]);
    }

    glBindVertexArray(Mesh->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, Mesh->VBO);

    glDrawArrays(GL_TRIANGLES, 0, Mesh->VerticesCount);
}

/*
    Creating the mesh
*/

//Note(Zen): Push different colours after this function
internal void
AddTriangleToHexMesh(hex_mesh * Mesh, v3 p0, v3 p1, v3 p2) {
    Assert((Mesh->VerticesCount <= MAX_HEX_VERTICES - 3));

    v3 Colour = v3(0.f, 1.f, 1.f);
    Mesh->Vertices[Mesh->VerticesCount++] = C3DVertex(p0, Colour, v2(0.1f, 0.1f), 0);
    Mesh->Vertices[Mesh->VerticesCount++] = C3DVertex(p1, Colour, v2(0.1f, 0.1f), 0);
    Mesh->Vertices[Mesh->VerticesCount++] = C3DVertex(p2, Colour, v2(0.1f, 0.1f), 0);
}

internal void
AddTriangleColour3(hex_mesh * Mesh, v3 c1, v3 c2, v3 c3) {
    Mesh->VerticesCount -= 3;
    Mesh->Vertices[Mesh->VerticesCount++].Colour = c1;
    Mesh->Vertices[Mesh->VerticesCount++].Colour = c2;
    Mesh->Vertices[Mesh->VerticesCount++].Colour = c3;

    #ifdef USING_HEX_INDICES
    Mesh->Indices[Mesh->IndicesCount] = Mesh->IndicesCount;
    Mesh->Indices[Mesh->IndicesCount + 1] = Mesh->IndicesCount + 1;
    Mesh->Indices[Mesh->IndicesCount + 2] = Mesh->IndicesCount + 2;
    Mesh->Indices += 3;
    #endif
}

internal void
AddTriangleColour(hex_mesh * Mesh, v3 Colour) {
    AddTriangleColour3(Mesh, Colour, Colour, Colour);
}

//TODO(Zen): Make this work with indices later on
internal void
AddQuadToHexMesh(hex_mesh * Mesh, v3 p0, v3 p1, v3 p2, v3 p3) {
    Assert(Mesh->VerticesCount <= MAX_HEX_VERTICES - 6);

    v3 Colour = v3(1.f, 1.f, 1.f);

    Mesh->Vertices[Mesh->VerticesCount++] = C3DVertex(p0, Colour, v2(0.1f, 0.1f), 0);
    Mesh->Vertices[Mesh->VerticesCount++] = C3DVertex(p1, Colour, v2(0.1f, 0.1f), 0);
    Mesh->Vertices[Mesh->VerticesCount++] = C3DVertex(p2, Colour, v2(0.1f, 0.1f), 0);

    Mesh->Vertices[Mesh->VerticesCount++] = C3DVertex(p3, Colour, v2(0.1f, 0.1f), 0);
    Mesh->Vertices[Mesh->VerticesCount++] = C3DVertex(p1, Colour, v2(0.1f, 0.1f), 0);
    Mesh->Vertices[Mesh->VerticesCount++] = C3DVertex(p2, Colour, v2(0.1f, 0.1f), 0);
}

internal void
TriangulateCell(hex_mesh * Mesh, hex_cell Cell) {
    v3 Center = Cell.Position;
    v3 Colour = Cell.Colour;
    for(int i = 0; i < 6; ++i) {
        i32 NextIndex = (i + 1) % 6;
        i32 PrevIndex = (i==0) ? 5 : i-1;
        //Note(Zen): Inner Triangles
        AddTriangleToHexMesh(Mesh, Center, CrestV3Add(Center, CrestV3Scale(HexCorners[i], HEX_SOLID_FACTOR)),
                                           CrestV3Add(Center, CrestV3Scale(HexCorners[NextIndex], HEX_SOLID_FACTOR)));

        hex_cell Neighbour0 = Cell.Neighbours[PrevIndex] ? *Cell.Neighbours[PrevIndex] : Cell;
        hex_cell Neighbour1 = Cell.Neighbours[i] ? *Cell.Neighbours[i] : Cell;
        hex_cell Neighbour2 = Cell.Neighbours[NextIndex] ? *Cell.Neighbours[NextIndex] : Cell;
        v3 EdgeColour1 = v3((Colour.x + Neighbour0.Colour.x + Neighbour1.Colour.x) * 0.3333333333f,
                            (Colour.y + Neighbour0.Colour.y + Neighbour1.Colour.y) * 0.3333333333f,
                            (Colour.z + Neighbour0.Colour.z + Neighbour1.Colour.z) * 0.3333333333f);

        v3 EdgeColour2 = v3((Colour.x + Neighbour1.Colour.x + Neighbour2.Colour.x) * 0.3333333333f,
                           (Colour.y + Neighbour1.Colour.y + Neighbour2.Colour.y) * 0.3333333333f,
                           (Colour.z + Neighbour1.Colour.z + Neighbour2.Colour.z) * 0.3333333333f);

        AddTriangleColour3(Mesh, Colour, EdgeColour1, EdgeColour2);
        //Note(Zen): Outer Quads
        v3 p0 = CrestV3Add(Center, CrestV3Scale(HexCorners[i], HEX_SOLID_FACTOR));
        v3 p1 = CrestV3Add(Center, HexCorners[i]);
        v3 p2 = CrestV3Add(Center, CrestV3Scale(HexCorners[NextIndex], HEX_SOLID_FACTOR));
        v3 p3 = CrestV3Add(Center, HexCorners[NextIndex]);

        //AddQuadToHexMesh(Mesh, p0, p1, p2, p3);
    }
}

internal void
TriangulateMesh(hex_grid * Grid) {
    //this will be the chunk's mesh later on
    hex_mesh * Mesh = &Grid->HexMesh;
    Mesh->VerticesCount = 0;
    for(i32 x = 0; x < HEX_CHUNK_WIDTH; ++x) {
        for(i32 z = 0; z < HEX_CHUNK_HEIGHT; ++z) {
            hex_cell Cell = Grid->Cells[z * HEX_CHUNK_WIDTH + x];
            TriangulateCell(Mesh, Cell);
        }
    }

    glUseProgram(Mesh->Shader);
    glBindVertexArray(Mesh->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, Mesh->VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, Mesh->VerticesCount * sizeof(C3DVertex), Mesh->Vertices);
}

internal hex_mesh
InitHexMesh(b32 ForEditor) {
    hex_mesh Mesh = {0};
    Mesh.Shader = CrestShaderInit("../assets/3D_vertex_shader.vs", "../assets/3D_fragment_shader.fs");

    glGenVertexArrays(1, &Mesh.VAO);
    glBindVertexArray(Mesh.VAO);


    glGenBuffers(1, &Mesh.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, Mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Mesh.Vertices), Mesh.Vertices, GL_DYNAMIC_DRAW);


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(C3DVertex), 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(C3DVertex), (void *)offsetof(C3DVertex, Colour));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(C3DVertex), (void *)offsetof(C3DVertex, TextureCoord));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(C3DVertex), (void *)offsetof(C3DVertex, TextureID));
    glEnableVertexAttribArray(3);

    return Mesh;
}
