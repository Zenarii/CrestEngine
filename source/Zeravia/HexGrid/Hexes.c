
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
    glUseProgram(Mesh->Shader);

    glBindTextureUnit(0, Renderer->Textures[0]);

    glBindVertexArray(Mesh->VAO);
    glDrawArrays(GL_TRIANGLES, 0, Mesh->VerticesCount);
}

/*
    Creating the mesh
*/

//Note(Zen): Push different colours after this function
internal void
AddTriangleToHexMesh(hex_mesh * Mesh, v3 p0, v3 p1, v3 p2) {
    Assert((Mesh->VerticesCount <= MAX_HEX_VERTICES - 3));

    v3 Colour = v3(1.f, 1.f, 1.f);

    v3 Edge1 = CrestV3Sub(p0, p1);
    v3 Edge2 = CrestV3Sub(p0, p2);

    v3 Normal = CrestV3Normalise(CrestV3Cross(Edge1, Edge2));

    Mesh->Vertices[Mesh->VerticesCount++] = HexMeshVertex(p0, Normal, Colour, v2(0.1f, 0.1f), 0);
    Mesh->Vertices[Mesh->VerticesCount++] = HexMeshVertex(p1, Normal, Colour, v2(0.1f, 0.1f), 0);
    Mesh->Vertices[Mesh->VerticesCount++] = HexMeshVertex(p2, Normal, Colour, v2(0.1f, 0.1f), 0);
}

internal void
AddTriangleColour3(hex_mesh * Mesh, v3 c1, v3 c2, v3 c3) {
    Mesh->VerticesCount -= 3;
    Mesh->Vertices[Mesh->VerticesCount++].Colour = c1;
    Mesh->Vertices[Mesh->VerticesCount++].Colour = c2;
    Mesh->Vertices[Mesh->VerticesCount++].Colour = c3;


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

    v3 Edge1 = CrestV3Sub(p0, p1);
    v3 Edge2 = CrestV3Sub(p0, p2);

    v3 Normal = CrestV3Normalise(CrestV3Cross(Edge1, Edge2));

    Mesh->Vertices[Mesh->VerticesCount++] = HexMeshVertex(p0, Normal, Colour, v2(0.1f, 0.1f), 0);
    Mesh->Vertices[Mesh->VerticesCount++] = HexMeshVertex(p1, Normal, Colour, v2(0.1f, 0.1f), 0);
    Mesh->Vertices[Mesh->VerticesCount++] = HexMeshVertex(p2, Normal, Colour, v2(0.1f, 0.1f), 0);

    Mesh->Vertices[Mesh->VerticesCount++] = HexMeshVertex(p3, Normal, Colour, v2(0.1f, 0.1f), 0);
    Mesh->Vertices[Mesh->VerticesCount++] = HexMeshVertex(p0, Normal, Colour, v2(0.1f, 0.1f), 0);
    Mesh->Vertices[Mesh->VerticesCount++] = HexMeshVertex(p2, Normal, Colour, v2(0.1f, 0.1f), 0);
}

internal void
AddQuadColour4(hex_mesh * Mesh, v3 c0, v3 c1, v3 c2, v3 c3) {
    Mesh->VerticesCount -= 6;
    Mesh->Vertices[Mesh->VerticesCount++].Colour = c0;
    Mesh->Vertices[Mesh->VerticesCount++].Colour = c1;
    Mesh->Vertices[Mesh->VerticesCount++].Colour = c2;
    Mesh->Vertices[Mesh->VerticesCount++].Colour = c3;
    Mesh->Vertices[Mesh->VerticesCount++].Colour = c0;
    Mesh->Vertices[Mesh->VerticesCount++].Colour = c2;
}

internal void
AddQuadColour2(hex_mesh * Mesh, v3 c01, v3 c23) {
    AddQuadColour4(Mesh, c01, c01, c23, c23);
}

internal void
AddQuadColour(hex_mesh * Mesh, v3 Colour) {
    AddQuadColour4(Mesh, Colour, Colour, Colour, Colour);
}

internal v3
TerracePositionLerp(v3 a, v3 b, u32 Step) {
    r32 Horizontal = Step * HEX_HORIZONTAL_TERRACE_SIZE;
    a.x += (b.x - a.x) * Horizontal;
    a.z += (b.z - a.z) * Horizontal;
    r32 Vertical = ((Step + 1)/2) * HEX_VERTICAL_TERRACE_SIZE;
    a.y += (b.y - a.y) * Vertical;
    return a;
}

internal v3
TerraceColourLerp(v3 a, v3 b, u32 Step) {
    r32 t = Step * HEX_HORIZONTAL_TERRACE_SIZE;
    a.x += (b.x - a.x) * t;
    a.y += (b.y - a.y) * t;
    a.z += (b.z - a.z) * t;
    return a;
}

internal v3
GetBridgeLocation(hex_direction Direction) {
    v3 Unscaled = CrestV3Add(HexCorners[Direction], HexCorners[Direction + 1]);
    return CrestV3Scale(Unscaled, HEX_BLEND_FACTOR);
}

internal void
TriangulateEdgeTerraces(hex_mesh * Mesh, v3 StartLeft, v3 StartRight, hex_cell StartCell,
 v3 EndLeft, v3 EndRight, hex_cell EndCell) {
     v3 p2 = TerracePositionLerp(StartRight, EndRight, 1);
     v3 p3 = TerracePositionLerp(StartLeft, EndLeft, 1);
     v3 c2 = TerraceColourLerp(StartCell.Colour, EndCell.Colour, 1);
     AddQuadToHexMesh(Mesh, StartLeft, StartRight, p2, p3);
     AddQuadColour2(Mesh, StartCell.Colour, c2);

     for(i32 i = 2; i < HEX_TERRACE_STEPS; ++i) {
         v3 p0 = p3;
         v3 p1 = p2;
         v3 c1 = c2;

         p2 = TerracePositionLerp(StartRight, EndRight, i);
         p3 = TerracePositionLerp(StartLeft, EndLeft, i);
         c2 = TerraceColourLerp(StartCell.Colour, EndCell.Colour, i);

         AddQuadToHexMesh(Mesh, p0, p1, p2, p3);
         AddQuadColour2(Mesh, c1, c2);
     }

     AddQuadToHexMesh(Mesh, p3, p2, EndRight, EndLeft);
     AddQuadColour2(Mesh, c2, EndCell.Colour);

}

internal void
TriangulateCornerTerraces(hex_mesh * Mesh, v3 Bottom, hex_cell BottomCell,
 v3 Left, hex_cell LeftCell, v3 Right, hex_cell RightCell) {
    v3 p2 = TerracePositionLerp(Bottom, Left, 1);
    v3 p3 = TerracePositionLerp(Bottom, Right, 1);
    v3 c2 = TerraceColourLerp(BottomCell.Colour, LeftCell.Colour, 1);
    v3 c3 = TerraceColourLerp(BottomCell.Colour, RightCell.Colour, 1);

    AddTriangleToHexMesh(Mesh, Bottom, p2, p3);
    AddTriangleColour3(Mesh, BottomCell.Colour, c2, c3);

    for (i32 i = 2; i < HEX_TERRACE_STEPS; ++i) {
        v3 p0 = p3;
        v3 p1 = p2;
        p2 = TerracePositionLerp(Bottom, Left, i);
        p3 = TerracePositionLerp(Bottom, Right, i);

        v3 c0 = c3;
        v3 c1 = c2;
        c2 = TerraceColourLerp(BottomCell.Colour, LeftCell.Colour, i);
        c3 = TerraceColourLerp(BottomCell.Colour, RightCell.Colour, i);

        AddQuadToHexMesh(Mesh, p0, p1, p2, p3);
        AddQuadColour4(Mesh, c0, c1, c2, c3);
    }

     AddQuadToHexMesh(Mesh, p3, p2, Left, Right);
     AddQuadColour4(Mesh, c3, c2, LeftCell.Colour, RightCell.Colour);
}

internal void
TriangulateBoundaryTriangle(hex_mesh * Mesh, v3 Bottom, hex_cell BottomCell,
 v3 Left, hex_cell LeftCell, v3 Boundary, v3 BoundaryColour) {
      v3 p1 = TerracePositionLerp(Bottom, Left, 1);
      v3 c1 = TerraceColourLerp(BottomCell.Colour, LeftCell.Colour, 1);


     AddTriangleToHexMesh(Mesh, Bottom, p1, Boundary);
     AddTriangleColour3(Mesh, BottomCell.Colour, c1, BoundaryColour);

     for(i32 i = 2; i < HEX_TERRACE_STEPS; ++i) {
         v3 p0 = p1;
         v3 c0 = c1;

         p1 = TerracePositionLerp(Bottom, Left, i);
         c1 = TerraceColourLerp(BottomCell.Colour, LeftCell.Colour, i);

         AddTriangleToHexMesh(Mesh, p0, p1, Boundary);
         AddTriangleColour3(Mesh, c0, c1, BoundaryColour);
     }

     AddTriangleToHexMesh(Mesh, p1, Left, Boundary);
     AddTriangleColour3(Mesh, c1, LeftCell.Colour, BoundaryColour);
}

//TODO(Zen): Instead of all the terraces goign to a point,
//keep them level up untill they reach the cliff
internal void
TriangulateCornerTerracesCliff(hex_mesh * Mesh, v3 Bottom, hex_cell BottomCell,
 v3 Left, hex_cell LeftCell, v3 Right, hex_cell RightCell) {
     r32 BoundaryScale = 1.f / (RightCell.Elevation - BottomCell.Elevation);
     BoundaryScale = (BoundaryScale > 0.f) ? BoundaryScale : - BoundaryScale;
     v3 Boundary = CrestV3Lerp(Bottom, Right, BoundaryScale);
     v3 BoundaryColour = CrestV3Lerp(BottomCell.Colour, RightCell.Colour, BoundaryScale);


     TriangulateBoundaryTriangle(Mesh, Bottom, BottomCell, Left, LeftCell, Boundary, BoundaryColour);

     if(GetHexEdgeType(LeftCell.Elevation, RightCell.Elevation) == HEX_EDGE_TERRACE) {
         TriangulateBoundaryTriangle(Mesh, Left, LeftCell, Right, RightCell, Boundary, BoundaryColour);
     }
     else {
         AddTriangleToHexMesh(Mesh, Left, Right, Boundary);
         AddTriangleColour3(Mesh, LeftCell.Colour, RightCell.Colour, BoundaryColour);
     }
}

internal void
TriangulateCornerCliffTerraces(hex_mesh * Mesh, v3 Bottom, hex_cell BottomCell,
 v3 Left, hex_cell LeftCell, v3 Right, hex_cell RightCell) {
     r32 BoundaryScale = 1.f / (LeftCell.Elevation - BottomCell.Elevation);
     BoundaryScale = (BoundaryScale > 0.f) ? BoundaryScale : - BoundaryScale;
     v3 Boundary = CrestV3Lerp(Bottom, Left, BoundaryScale);
     v3 BoundaryColour = CrestV3Lerp(BottomCell.Colour, LeftCell.Colour, BoundaryScale);

     TriangulateBoundaryTriangle(Mesh, Right, RightCell, Bottom, BottomCell, Boundary, BoundaryColour);

     if(GetHexEdgeType(RightCell.Elevation, LeftCell.Elevation) == HEX_EDGE_TERRACE) {
         TriangulateBoundaryTriangle(Mesh, Left, LeftCell, Right, RightCell, Boundary, BoundaryColour);
     }
     else {
         AddTriangleToHexMesh(Mesh, Left, Right, Boundary);
         AddTriangleColour3(Mesh, LeftCell.Colour, RightCell.Colour, BoundaryColour);
     }

}

internal void
TriangulateCorner(hex_mesh * Mesh, v3 Bottom, hex_cell BottomCell,
 v3 Left, hex_cell LeftCell, v3 Right, hex_cell RightCell) {
    hex_edge_type RightEdgeType = GetHexEdgeType(BottomCell.Elevation, RightCell.Elevation);
    hex_edge_type LeftEdgeType = GetHexEdgeType(BottomCell.Elevation, LeftCell.Elevation);

    if(LeftEdgeType == HEX_EDGE_TERRACE) {
        if(RightEdgeType == HEX_EDGE_TERRACE) {
            TriangulateCornerTerraces(Mesh, Bottom, BottomCell, Left, LeftCell, Right, RightCell);
            return;
        }
        else if(RightEdgeType == HEX_EDGE_FLAT) {
            TriangulateCornerTerraces(Mesh, Left, LeftCell, Right, RightCell, Bottom, BottomCell);
            return;
        }
        else {
            TriangulateCornerTerracesCliff(Mesh, Bottom, BottomCell, Left, LeftCell, Right, RightCell);
            return;
        }
    }

    else if(RightEdgeType == HEX_EDGE_TERRACE) {
        if(LeftEdgeType == HEX_EDGE_FLAT) {
            TriangulateCornerTerraces(Mesh, Right, RightCell, Bottom, BottomCell, Left, LeftCell);
            return;
        }
        else {
            TriangulateCornerCliffTerraces(Mesh, Bottom, BottomCell, Left, LeftCell, Right, RightCell);
            return;
        }
    }

    else if(GetHexEdgeType(LeftCell.Elevation, RightCell.Elevation) == HEX_EDGE_TERRACE) {
        if(LeftCell.Elevation < RightCell.Elevation) {
            TriangulateCornerCliffTerraces(Mesh, Right, RightCell, Bottom, BottomCell, Left, LeftCell);
            return;
        }
        else {
            TriangulateCornerTerracesCliff(Mesh, Left, LeftCell, Right, RightCell, Bottom, BottomCell);
            return;
        }
    }


    AddTriangleToHexMesh(Mesh, Bottom, Left, Right);
    AddTriangleColour3(Mesh, BottomCell.Colour, LeftCell.Colour, RightCell.Colour);
}

internal void
TriangulateConnection(hex_mesh * Mesh, hex_cell Cell, hex_direction Direction, v3 p0, v3 p1) {
    if(Direction < HEX_DIRECTION_NW) {
        hex_direction NextDirection = Direction + 1;
        if(Cell.Neighbours[Direction]) {
            hex_cell Neighbour = *Cell.Neighbours[Direction];

            v3 Bridge = GetBridgeLocation(Direction);
            v3 p2 = CrestV3Add(p1, Bridge);
            v3 p3 = CrestV3Add(p0, Bridge);

            p2.y = p3.y = Neighbour.Elevation * HEX_ELEVATION_STEP;
            if(GetHexEdgeType(Cell.Elevation, Neighbour.Elevation) == HEX_EDGE_TERRACE) {
                TriangulateEdgeTerraces(Mesh, p1, p0, Cell, p2, p3, Neighbour);
            }
            else {
                AddQuadToHexMesh(Mesh, p1, p0, p3, p2);
                AddQuadColour2(Mesh, Cell.Colour, Neighbour.Colour);
            }

            if(Direction <= 2 && Cell.Neighbours[NextDirection]) {
                hex_cell NextNeighbour = *Cell.Neighbours[NextDirection];

                v3 p4 = CrestV3Add(p1, GetBridgeLocation(Direction + 1));
                p4.y = NextNeighbour.Elevation * HEX_ELEVATION_STEP;
                //Note(Zen): Deduce orientation
                if(Cell.Elevation <= Neighbour.Elevation) {
                    if(Cell.Elevation <= NextNeighbour.Elevation) {
                        TriangulateCorner(Mesh, p1, Cell, p2, Neighbour, p4, NextNeighbour);
                    }
                    else {
                        TriangulateCorner(Mesh, p4, NextNeighbour, p1, Cell, p2, Neighbour);
                    }
                }
                else if(Neighbour.Elevation <= NextNeighbour.Elevation) {
                    TriangulateCorner(Mesh, p2, Neighbour, p4, NextNeighbour, p1, Cell);
                }
                else {
                    TriangulateCorner(Mesh, p4, NextNeighbour, p1, Cell, p2, Neighbour);
                }
            }
        }
    }
}



internal void
TriangulateCell(hex_mesh * Mesh, hex_cell Cell) {
    v3 Center = Cell.Position;
    v3 Colour = Cell.Colour;
    for(i32 Direction = 0; Direction < 6; ++Direction) {
        i32 NextDirection = (Direction + 1) % 6;

        //Note(Zen): Inner Triangles
        v3 p0 = CrestV3Add(Center, CrestV3Scale(HexCorners[Direction], HEX_SOLID_FACTOR));
        v3 p1 = CrestV3Add(Center, CrestV3Scale(HexCorners[NextDirection], HEX_SOLID_FACTOR));
        AddTriangleToHexMesh(Mesh, Center, p0, p1);
        AddTriangleColour(Mesh, Colour);

        //Note(Zen): Bridges
        TriangulateConnection(Mesh, Cell, Direction, p0, p1);
        //Note(Zen): Corners


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
    glBufferSubData(GL_ARRAY_BUFFER, 0, Mesh->VerticesCount * sizeof(hex_mesh_vertex), Mesh->Vertices);

}

internal hex_mesh
InitHexMesh(b32 ForEditor) {
    hex_mesh Mesh = {0};
    //Probably a way to make all the meshes use the same shader
    //May not matter, in the game state map will be one whole opaque mesh
    Mesh.Shader = CrestShaderInit("../assets/hex_shader.vs", "../assets/hex_shader.fs");
    {
        glGenVertexArrays(1, &Mesh.VAO);
        glBindVertexArray(Mesh.VAO);


        glGenBuffers(1, &Mesh.VBO);
        glBindBuffer(GL_ARRAY_BUFFER, Mesh.VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Mesh.Vertices), Mesh.Vertices, GL_DYNAMIC_DRAW);

        //Position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(hex_mesh_vertex), 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(hex_mesh_vertex), (void*)offsetof(hex_mesh_vertex, Normal));
        glEnableVertexAttribArray(1);

        //Colour
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(hex_mesh_vertex), (void *)offsetof(hex_mesh_vertex, Colour));
        glEnableVertexAttribArray(2);

        //Texture
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(hex_mesh_vertex), (void *)offsetof(hex_mesh_vertex, TextureCoord));
        glEnableVertexAttribArray(3);

        //Texture ID
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(hex_mesh_vertex), (void *)offsetof(hex_mesh_vertex, TextureID));
        glEnableVertexAttribArray(4);
    }

    //Note(Zen): Set up Texture Array
    {
        glUseProgram(Mesh.Shader);
        i32 Location = glGetUniformLocation(Mesh.Shader, "Images");
        int samplers[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
        glUniform1iv(Location, 16, samplers);
    }

    return Mesh;
}

internal collision_mesh
CollisionMeshFromHexMesh(hex_mesh * Mesh) {
    collision_mesh Result = {0};
    u32 Index = 0;
    for(; 3 * Index < Mesh->VerticesCount; ++Index) {
        Result.Triangles[Index] = CreateTriangle(Mesh->Vertices[3 * Index].Position,
                                                 Mesh->Vertices[3 * Index + 1].Position,
                                                 Mesh->Vertices[3 * Index + 2].Position);
    }
    Result.TriangleCount = Index;
    return Result;
}
