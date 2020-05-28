internal void
HexGridInit(hex_grid * Grid) {
    if(!Grid->Initialised){
        Grid->MeshShader = &App->Shaders.Terrain;
        glUseProgram(Grid->MeshShader->ShaderID);
        i32 Location = glGetUniformLocation(Grid->MeshShader->ShaderID, "Images");
        int samplers[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
        glUniform1iv(Location, 16, samplers);

        Grid->MeshTexture = CasLoadTexture("assets/White.png", GL_LINEAR);

        Assert(App->Shaders.Water.Loaded);
        Grid->WaterShader = &App->Shaders.Water;

        CrestShaderSetInt(Grid->WaterShader->ShaderID, "ReflectionTexture", 0);
        CrestShaderSetInt(Grid->WaterShader->ShaderID, "ReflectionTexture", 1);
        CrestShaderSetInt(Grid->WaterShader->ShaderID, "DistortionTexture", 2);
        CrestShaderSetInt(Grid->WaterShader->ShaderID, "DepthMap", 3);

        Grid->WaterDistortionTexture = CasLoadTexture("assets/Textures/WaterDuDv.png", GL_LINEAR);

        Grid->RefractionFBO = CrestCreateFramebuffer(App->ScreenWidth, App->ScreenHeight, 1);
        Grid->ReflectionFBO = CrestCreateFramebuffer(App->ScreenWidth, App->ScreenHeight, 0);

        Grid->Width = HEX_MAX_WIDTH_IN_CELLS;
        Grid->Height = HEX_CHUNK_HEIGHT * HEX_MAX_CHUNKS_HIGH;
        InitFeatureSet(&Grid->FeatureSet);

        Grid->Initialised = 1;
    }
}

internal hex_cell
CreateCell(int x, int z) {
    hex_cell Result = {0};

    Result.Position = v3((x + z * 0.5f - z/2) * HEX_INNER_DIAMETER, 0.f, z * HEX_OUTER_RADIUS * 1.5f);
    v3 Sample = Noise3DSample(Result.Position);
    Result.Position.y += Sample.y * HEX_ELEVATION_NUDGE_STRENGTH;

    return Result;
}

internal void
AddNeighboursToCell(hex_grid * Grid, hex_cell * Cell, i32 x, i32 z) {
    i32 Index = z * HEX_MAX_WIDTH_IN_CELLS + x;
    if(x > 0) {
        Cell->Neighbours[HEX_DIRECTION_W] = &Grid->Cells[Index - 1];
        Grid->Cells[Index - 1].Neighbours[HEX_DIRECTION_E] = Cell;
    }
    if(z > 0) {
        if((z % 2) == 0) {
            Cell->Neighbours[HEX_DIRECTION_NE] = &Grid->Cells[Index - HEX_MAX_WIDTH_IN_CELLS];
            Grid->Cells[Index - HEX_MAX_WIDTH_IN_CELLS].Neighbours[HEX_DIRECTION_SW] = Cell;

            if(x > 0) {
                Cell->Neighbours[HEX_DIRECTION_NW] = &Grid->Cells[Index - HEX_MAX_WIDTH_IN_CELLS - 1];
                Grid->Cells[Index - HEX_MAX_WIDTH_IN_CELLS - 1].Neighbours[HEX_DIRECTION_SE] = Cell;
            }
        }
        else {
            Cell->Neighbours[HEX_DIRECTION_NW] = &Grid->Cells[Index - HEX_MAX_WIDTH_IN_CELLS];
            Grid->Cells[Index - HEX_MAX_WIDTH_IN_CELLS].Neighbours[HEX_DIRECTION_SE] = Cell;

            if(x < Grid->Width - 1) {
                Cell->Neighbours[HEX_DIRECTION_NE] = &Grid->Cells[Index - HEX_MAX_WIDTH_IN_CELLS + 1];
                Grid->Cells[Index - HEX_MAX_WIDTH_IN_CELLS + 1].Neighbours[HEX_DIRECTION_SW] = Cell;
            }
        }
    }
}


internal void
ResetCellsOnHexGrid(hex_grid * Grid) {
    for(i32 z = 0; z < Grid->Height; ++z) {
        for(i32 x = 0; x < Grid->Width; ++x) {

            i32 Index = z * HEX_MAX_WIDTH_IN_CELLS + x;
            Grid->Cells[Index] = CreateCell(x, z);

            hex_cell * Cell = &Grid->Cells[Index];
            Cell->Index = Index;

            AddNeighboursToCell(Grid, Cell, x, z);
        }
    }
    for(hex_feature_type Type = 1; Type < HEX_FEATURE_COUNT; ++Type) {
        memset(Grid->FeatureSet.Features[Type].Model, 0, sizeof(Grid->FeatureSet.Features[Type].Model));
    }
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
    u32 Result = Coords.z * HEX_MAX_WIDTH_IN_CELLS + Coords.x + (Coords.z/2);
    if(Result > HEX_MAX_CHUNKS * HEX_CHUNK_WIDTH * HEX_CHUNK_HEIGHT) return -1;

    return Result;
}

internal i32
GetChunkIndexFromCellIndex(i32 CellIndex) {
    i32 CellPositionInRow = CellIndex % HEX_MAX_WIDTH_IN_CELLS;
    i32 CellPositionInColumn = CellIndex / HEX_MAX_WIDTH_IN_CELLS;

    i32 ChunkX = CellPositionInRow / HEX_CHUNK_WIDTH;
    i32 ChunkZ = CellPositionInColumn / HEX_CHUNK_HEIGHT;

    return ChunkZ * HEX_MAX_CHUNKS_WIDE + ChunkX;
}

/*
Collisions Calculations:
*/

internal collision_triangle
CreateTriangle(v3 v0, v3 v1, v3 v2) {
    collision_triangle Result = {v0, v1, v2};
    return Result;
};

/*
    Drawing the Mesh to the screen
*/
internal void
PreDrawHexMesh(hex_grid * Grid) {
    glUseProgram(Grid->MeshShader->ShaderID);
    glBindTextureUnit(0, Grid->MeshTexture);
}

internal void
DrawHexMesh(hex_grid * Grid, hex_mesh * Mesh) {
    glBindVertexArray(Mesh->VAO);
    glDrawArrays(GL_TRIANGLES, 0, Mesh->VerticesCount);
}

internal void
PreDrawWaterMesh(hex_grid * Grid) {
    glUseProgram(Grid->WaterShader->ShaderID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Grid->ReflectionFBO.Texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, Grid->RefractionFBO.Texture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, Grid->WaterDistortionTexture);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, Grid->RefractionFBO.DepthTexture);
}

internal void
DrawWaterMesh(hex_grid * Grid, hex_mesh * Mesh) {
    glBindVertexArray(Mesh->VAO);
    glDrawArrays(GL_TRIANGLES, 0, Mesh->VerticesCount);
}

/*
    Creating the mesh
*/

internal hex_edge_vertices
HexEdgeVertices(v3 p0, v3 p1) {
    hex_edge_vertices Result = {0};
    Result.p0 = p0;
    Result.p1 = CrestV3Lerp(p0, p1, 1.f/3.f);
    Result.p2 = CrestV3Lerp(p0, p1, 2.f/3.f);
    Result.p3 = p1;
    return Result;
}

internal v3
NudgeVertex(v3 Vertex) {
    v3 ScaledVertex = CrestV3Scale(Vertex, HEX_NOISE_SCALE);
    v3 Sample = Noise3DSample(ScaledVertex);
    Sample = CrestV3Scale(Sample, HEX_NUDGE_STRENGTH);
    //Note(Zen): Don't change heights so cell tops flat
    Sample.y = 0.f;
    v3 Result = CrestV3Add(Vertex, Sample);
    return Result;
}

//Note(Zen): Push different colours after this function
internal void
AddTriangleToHexMeshUnnudged(temporary_hex_mesh * Mesh, v3 p0, v3 p1, v3 p2) {
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
AddTriangleToHexMesh(temporary_hex_mesh * Mesh, v3 p0, v3 p1, v3 p2) {
    p0 = NudgeVertex(p0);
    p1 = NudgeVertex(p1);
    p2 = NudgeVertex(p2);

    AddTriangleToHexMeshUnnudged(Mesh, p0, p1, p2);
}

internal void
AddTriangleColour3(temporary_hex_mesh * Mesh, v3 c1, v3 c2, v3 c3) {
    Mesh->VerticesCount -= 3;
    Mesh->Vertices[Mesh->VerticesCount++].Colour = c1;
    Mesh->Vertices[Mesh->VerticesCount++].Colour = c2;
    Mesh->Vertices[Mesh->VerticesCount++].Colour = c3;
}

internal void
AddTriangleColour(temporary_hex_mesh * Mesh, v3 Colour) {
    AddTriangleColour3(Mesh, Colour, Colour, Colour);
}

internal void
AddTriangleUVs3(temporary_hex_mesh * Mesh, v2 uv1, v2 uv2, v2 uv3) {
    Mesh->VerticesCount -= 3;
    Mesh->Vertices[Mesh->VerticesCount++].TextureCoord = uv1;
    Mesh->Vertices[Mesh->VerticesCount++].TextureCoord = uv2;
    Mesh->Vertices[Mesh->VerticesCount++].TextureCoord = uv3;
}

internal void
AddTriangleUVs(temporary_hex_mesh * Mesh, v2 uv) {
    AddTriangleUVs3(Mesh, uv, uv, uv);
}
internal void
AddQuadToHexMeshUnnudged(temporary_hex_mesh * Mesh, v3 p0, v3 p1, v3 p2, v3 p3) {
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
AddQuadToHexMesh(temporary_hex_mesh * Mesh, v3 p0, v3 p1, v3 p2, v3 p3) {
    p0 = NudgeVertex(p0);
    p1 = NudgeVertex(p1);
    p2 = NudgeVertex(p2);
    p3 = NudgeVertex(p3);

    AddQuadToHexMeshUnnudged(Mesh, p0, p1, p2, p3);
}

internal void
AddQuadColour4(temporary_hex_mesh * Mesh, v3 c0, v3 c1, v3 c2, v3 c3) {
    Mesh->VerticesCount -= 6;
    Mesh->Vertices[Mesh->VerticesCount++].Colour = c0;
    Mesh->Vertices[Mesh->VerticesCount++].Colour = c1;
    Mesh->Vertices[Mesh->VerticesCount++].Colour = c2;
    Mesh->Vertices[Mesh->VerticesCount++].Colour = c3;
    Mesh->Vertices[Mesh->VerticesCount++].Colour = c0;
    Mesh->Vertices[Mesh->VerticesCount++].Colour = c2;
}

internal void
AddQuadColour2(temporary_hex_mesh * Mesh, v3 c01, v3 c23) {
    AddQuadColour4(Mesh, c01, c01, c23, c23);
}

internal void
AddQuadColour(temporary_hex_mesh * Mesh, v3 Colour) {
    AddQuadColour4(Mesh, Colour, Colour, Colour, Colour);
}

internal void
AddQuadUvs4(temporary_hex_mesh * Mesh, v2 uv0, v2 uv1, v2 uv2, v2 uv3) {
    Mesh->VerticesCount -= 6;
    Mesh->Vertices[Mesh->VerticesCount++].TextureCoord = uv0;
    Mesh->Vertices[Mesh->VerticesCount++].TextureCoord = uv1;
    Mesh->Vertices[Mesh->VerticesCount++].TextureCoord = uv2;
    Mesh->Vertices[Mesh->VerticesCount++].TextureCoord = uv3;
    Mesh->Vertices[Mesh->VerticesCount++].TextureCoord = uv0;
    Mesh->Vertices[Mesh->VerticesCount++].TextureCoord = uv2;
}

internal void
AddQuadUvs(temporary_hex_mesh * Mesh, v2 uv) {
    AddQuadUvs4(Mesh, uv, uv, uv, uv);
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

internal hex_edge_vertices
TerraceEdgePositionLerp(hex_edge_vertices e0, hex_edge_vertices e1, u32 Step) {
    hex_edge_vertices Result = {0};
    Result.p0 = TerracePositionLerp(e0.p0, e1.p0, Step);
    Result.p1 = TerracePositionLerp(e0.p1, e1.p1, Step);
    Result.p2 = TerracePositionLerp(e0.p2, e1.p2, Step);
    Result.p3 = TerracePositionLerp(e0.p3, e1.p3, Step);
    return Result;
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
TriangulateEdgeFan(temporary_hex_mesh * Mesh, v3 Center, hex_edge_vertices Edge, v3 Colour) {
    AddTriangleToHexMesh(Mesh, Center, Edge.p0, Edge.p1);
    AddTriangleColour(Mesh, Colour);
    AddTriangleToHexMesh(Mesh, Center, Edge.p1, Edge.p2);
    AddTriangleColour(Mesh, Colour);
    AddTriangleToHexMesh(Mesh, Center, Edge.p2, Edge.p3);
    AddTriangleColour(Mesh, Colour);
}

internal void
TriangulateEdgeStrip(temporary_hex_mesh * Mesh, hex_edge_vertices Edge0, v3 Colour0, hex_edge_vertices Edge1,  v3 Colour1) {
    AddQuadToHexMesh(Mesh, Edge0.p1, Edge0.p0, Edge1.p0, Edge1.p1);
    AddQuadColour2(Mesh, Colour0, Colour1);
    AddQuadToHexMesh(Mesh, Edge0.p2, Edge0.p1, Edge1.p1, Edge1.p2);
    AddQuadColour2(Mesh, Colour0, Colour1);
    AddQuadToHexMesh(Mesh, Edge0.p3, Edge0.p2, Edge1.p2, Edge1.p3);
    AddQuadColour2(Mesh, Colour0, Colour1);
}

internal void
TriangulateEdgeTerraces(temporary_hex_mesh * Mesh, hex_edge_vertices StartEdge, hex_cell StartCell, hex_edge_vertices EndEdge, hex_cell EndCell) {
     hex_edge_vertices Edge1 = TerraceEdgePositionLerp(StartEdge, EndEdge, 1);

     v3 StartColour = HexColours[StartCell.ColourIndex];
     v3 EndColour = HexColours[EndCell.ColourIndex];

     v3 c1 = TerraceColourLerp(StartColour, EndColour, 1);

     TriangulateEdgeStrip(Mesh, StartEdge, StartColour, Edge1, c1);

     for(i32 i = 2; i < HEX_TERRACE_STEPS; ++i) {
         hex_edge_vertices Edge0 = Edge1;
         v3 c0 = c1;

         Edge1 = TerraceEdgePositionLerp(StartEdge, EndEdge, i);
         c1 = TerraceColourLerp(StartColour, EndColour, i);

         TriangulateEdgeStrip(Mesh, Edge0, c0, Edge1, c1);
     }

     TriangulateEdgeStrip(Mesh, Edge1, c1, EndEdge, EndColour);
}

internal void
TriangulateCornerTerraces(temporary_hex_mesh * Mesh, v3 Bottom, hex_cell BottomCell,
 v3 Left, hex_cell LeftCell, v3 Right, hex_cell RightCell) {
    v3 BottomColour = HexColours[BottomCell.ColourIndex];
    v3 LeftColour = HexColours[LeftCell.ColourIndex];
    v3 RightColour = HexColours[RightCell.ColourIndex];



    v3 p2 = TerracePositionLerp(Bottom, Left, 1);
    v3 p3 = TerracePositionLerp(Bottom, Right, 1);
    v3 c2 = TerraceColourLerp(BottomColour, LeftColour, 1);
    v3 c3 = TerraceColourLerp(BottomColour, RightColour, 1);

    AddTriangleToHexMesh(Mesh, Bottom, p2, p3);
    AddTriangleColour3(Mesh, BottomColour, c2, c3);

    for (i32 i = 2; i < HEX_TERRACE_STEPS; ++i) {
        v3 p0 = p3;
        v3 p1 = p2;
        p2 = TerracePositionLerp(Bottom, Left, i);
        p3 = TerracePositionLerp(Bottom, Right, i);

        v3 c0 = c3;
        v3 c1 = c2;
        c2 = TerraceColourLerp(BottomColour, LeftColour, i);
        c3 = TerraceColourLerp(BottomColour, RightColour, i);

        AddQuadToHexMesh(Mesh, p0, p1, p2, p3);
        AddQuadColour4(Mesh, c0, c1, c2, c3);
    }

     AddQuadToHexMesh(Mesh, p3, p2, Left, Right);
     AddQuadColour4(Mesh, c3, c2, LeftColour, RightColour);
}

internal void
TriangulateBoundaryTriangle(temporary_hex_mesh * Mesh, v3 Bottom, hex_cell BottomCell,
 v3 Left, hex_cell LeftCell, v3 Boundary, v3 BoundaryColour) {
    v3 BottomColour = HexColours[BottomCell.ColourIndex];
    v3 LeftColour = HexColours[LeftCell.ColourIndex];


    v3 p1 = TerracePositionLerp(Bottom, Left, 1);
    v3 c1 = TerraceColourLerp(BottomColour, LeftColour, 1);

    AddTriangleToHexMeshUnnudged(Mesh, NudgeVertex(Bottom), NudgeVertex(p1), Boundary);
    AddTriangleColour3(Mesh, BottomColour, c1, BoundaryColour);

     for(i32 i = 2; i < HEX_TERRACE_STEPS; ++i) {
         v3 p0 = p1;
         v3 c0 = c1;

         p1 = TerracePositionLerp(Bottom, Left, i);
         c1 = TerraceColourLerp(BottomColour, LeftColour, i);

         AddTriangleToHexMeshUnnudged(Mesh, NudgeVertex(p0), NudgeVertex(p1), Boundary);
         AddTriangleColour3(Mesh, c0, c1, BoundaryColour);
     }

     AddTriangleToHexMeshUnnudged(Mesh, NudgeVertex(p1), NudgeVertex(Left), Boundary);
     AddTriangleColour3(Mesh, c1, LeftColour, BoundaryColour);
}

//TODO(Zen): Instead of all the terraces goign to a point,
//keep them level up untill they reach the cliff
internal void
TriangulateCornerTerracesCliff(temporary_hex_mesh * Mesh, v3 Bottom, hex_cell BottomCell,
 v3 Left, hex_cell LeftCell, v3 Right, hex_cell RightCell) {
    v3 BottomColour = HexColours[BottomCell.ColourIndex];
    v3 RightColour = HexColours[RightCell.ColourIndex];
    v3 LeftColour = HexColours[LeftCell.ColourIndex];

    r32 BoundaryScale = 1.f / (RightCell.Elevation - BottomCell.Elevation);
    BoundaryScale = (BoundaryScale > 0.f) ? BoundaryScale : - BoundaryScale;
    v3 Boundary = CrestV3Lerp(NudgeVertex(Bottom), NudgeVertex(Right), BoundaryScale);
    v3 BoundaryColour = CrestV3Lerp(BottomColour, RightColour, BoundaryScale);


    TriangulateBoundaryTriangle(Mesh, Bottom, BottomCell, Left, LeftCell, Boundary, BoundaryColour);

    if(GetHexEdgeType(LeftCell.Elevation, RightCell.Elevation) == HEX_EDGE_TERRACE) {
        TriangulateBoundaryTriangle(Mesh, Left, LeftCell, Right, RightCell, Boundary, BoundaryColour);
    }
    else {
        AddTriangleToHexMeshUnnudged(Mesh, NudgeVertex(Left), NudgeVertex(Right), Boundary);
        AddTriangleColour3(Mesh, LeftColour, RightColour, BoundaryColour);
    }
}

internal void
TriangulateCornerCliffTerraces(temporary_hex_mesh * Mesh, v3 Bottom, hex_cell BottomCell,
 v3 Left, hex_cell LeftCell, v3 Right, hex_cell RightCell) {
    v3 BottomColour = HexColours[BottomCell.ColourIndex];
    v3 RightColour = HexColours[RightCell.ColourIndex];
    v3 LeftColour = HexColours[LeftCell.ColourIndex];

    r32 BoundaryScale = 1.f / (LeftCell.Elevation - BottomCell.Elevation);
    BoundaryScale = (BoundaryScale > 0.f) ? BoundaryScale : - BoundaryScale;

    v3 Boundary = CrestV3Lerp(NudgeVertex(Bottom), NudgeVertex(Left), BoundaryScale);
    v3 BoundaryColour = CrestV3Lerp(BottomColour, LeftColour, BoundaryScale);

    TriangulateBoundaryTriangle(Mesh, Right, RightCell, Bottom, BottomCell, Boundary, BoundaryColour);

    if(GetHexEdgeType(RightCell.Elevation, LeftCell.Elevation) == HEX_EDGE_TERRACE) {
        TriangulateBoundaryTriangle(Mesh, Left, LeftCell, Right, RightCell, Boundary, BoundaryColour);
    }
    else {
        AddTriangleToHexMeshUnnudged(Mesh, NudgeVertex(Left), NudgeVertex(Right), Boundary);
        AddTriangleColour3(Mesh, LeftColour, RightColour, BoundaryColour);
    }
}

internal void
TriangulateCorner(temporary_hex_mesh * Mesh, v3 Bottom, hex_cell BottomCell,
 v3 Left, hex_cell LeftCell, v3 Right, hex_cell RightCell) {
    v3 BottomColour = HexColours[BottomCell.ColourIndex];
    v3 RightColour = HexColours[RightCell.ColourIndex];
    v3 LeftColour = HexColours[LeftCell.ColourIndex];

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
    AddTriangleColour3(Mesh, BottomColour, LeftColour, RightColour);
}

internal void
TriangulateConnection(temporary_hex_mesh * Mesh, hex_cell Cell, hex_direction Direction, hex_edge_vertices Edge0) {
    if(Direction < HEX_DIRECTION_NW) {
        hex_direction NextDirection = Direction + 1;
        if(Cell.Neighbours[Direction]) {
            hex_cell Neighbour = *Cell.Neighbours[Direction];

            v3 Bridge = GetBridgeLocation(Direction);
            Bridge.y = Neighbour.Position.y - Cell.Position.y;

            hex_edge_vertices Edge1 = HexEdgeVertices(
                CrestV3Add(Edge0.p0, Bridge),
                CrestV3Add(Edge0.p3, Bridge)
            );

            v3 CellColour = HexColours[Cell.ColourIndex];
            v3 NeighbourColour = HexColours[Neighbour.ColourIndex];

            if(GetHexEdgeType(Cell.Elevation, Neighbour.Elevation) == HEX_EDGE_TERRACE) {
                TriangulateEdgeTerraces(Mesh, Edge0, Cell, Edge1, Neighbour);
            }
            else {
                TriangulateEdgeStrip(Mesh, Edge0, CellColour, Edge1, NeighbourColour);
            }

            if(Direction <= 2 && Cell.Neighbours[NextDirection]) {
                hex_cell NextNeighbour = *Cell.Neighbours[NextDirection];

                v3 p4 = CrestV3Add(Edge0.p3, GetBridgeLocation(Direction + 1));
                p4.y = NextNeighbour.Position.y;
                //Note(Zen): Deduce orientation
                if(Cell.Elevation <= Neighbour.Elevation) {
                    if(Cell.Elevation <= NextNeighbour.Elevation) {
                        TriangulateCorner(Mesh, Edge0.p3, Cell, Edge1.p3, Neighbour, p4, NextNeighbour);
                    }
                    else {
                        TriangulateCorner(Mesh, p4, NextNeighbour, Edge0.p3, Cell, Edge1.p3, Neighbour);
                    }
                }
                else if(Neighbour.Elevation <= NextNeighbour.Elevation) {
                    TriangulateCorner(Mesh, Edge1.p3, Neighbour, p4, NextNeighbour, Edge0.p3, Cell);
                }
                else {
                    TriangulateCorner(Mesh, p4, NextNeighbour, Edge0.p3, Cell, Edge1.p3, Neighbour);
                }
            }

        }
    }
}


internal void
TriangulateCell(temporary_hex_mesh * Mesh, hex_cell Cell) {
    v3 Center = Cell.Position;
    v3 Colour = HexColours[Cell.ColourIndex];
    for(i32 Direction = 0; Direction < 6; ++Direction) {
        i32 NextDirection = (Direction + 1) % 6;

        hex_edge_vertices Edge = HexEdgeVertices(
            CrestV3Add(Center, CrestV3Scale(HexCorners[Direction], HEX_SOLID_FACTOR)),
            CrestV3Add(Center, CrestV3Scale(HexCorners[NextDirection], HEX_SOLID_FACTOR))
        );

        TriangulateEdgeFan(Mesh, Center, Edge, Colour);


        //Note(Zen): Bridges
        TriangulateConnection(Mesh, Cell, Direction, Edge);
    }
}


internal void
TriangulateMesh(hex_grid * Grid, hex_grid_chunk * Chunk) {
    //this will be the chunk's mesh later on
    hex_mesh * Mesh = &Chunk->HexMesh;
    temporary_hex_mesh TempMesh = {0};
    for(i32 x = Chunk->X * HEX_CHUNK_WIDTH; x < HEX_CHUNK_WIDTH * (Chunk->X + 1); ++x) {
        for(i32 z = Chunk->Z * HEX_CHUNK_HEIGHT; z < HEX_CHUNK_HEIGHT * (Chunk->Z + 1); ++z) {
            hex_cell Cell = Grid->Cells[z * HEX_CHUNK_WIDTH * HEX_MAX_CHUNKS_HIGH + x];
            TriangulateCell(&TempMesh, Cell);
        }
    }

    glBindVertexArray(Mesh->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, Mesh->VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, TempMesh.VerticesCount * sizeof(hex_mesh_vertex), TempMesh.Vertices);

    Mesh->VerticesCount = TempMesh.VerticesCount;
}

internal void
InitHexMesh(hex_mesh * Mesh) {
    glGenVertexArrays(1, &Mesh->VAO);
    glBindVertexArray(Mesh->VAO);


    glGenBuffers(1, &Mesh->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, Mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(hex_mesh_vertex) * MAX_HEX_VERTICES, 0, GL_DYNAMIC_DRAW);

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

/*
    Generating Water Meshes
*/

internal void
TriangulateWaterConnection(temporary_hex_mesh * Mesh, hex_cell Cell, hex_direction Direction, v3 p0, v3 p1) {
    if(Cell.Neighbours[Direction] && Direction <= HEX_DIRECTION_NE) {
        hex_cell Neighbour = *Cell.Neighbours[Direction];
        if(Neighbour.WaterLevel <= Neighbour.Elevation) return;
        v3 Bridge = GetBridgeLocation(Direction);

        v3 p2 = CrestV3Add(p1, Bridge);
        v3 p3 = CrestV3Add(p0, Bridge);

        v3 n0 = NudgeVertex(p0);
        v3 n1 = NudgeVertex(p1);
        v3 n2 = NudgeVertex(p2);
        v3 n3 = NudgeVertex(p3);

        n1.y = n0.y = (Cell.WaterLevel + HEX_WATER_ELEVATION_OFFSET) * HEX_ELEVATION_STEP;
        n2.y = n3.y = (Neighbour.WaterLevel + HEX_WATER_ELEVATION_OFFSET) * HEX_ELEVATION_STEP;
        AddQuadToHexMeshUnnudged(Mesh, n1, n0, n3, n2);
        AddQuadColour(Mesh, HEX_WATER_COLOUR);
        AddQuadUvs(Mesh, v2(0.f, 0.f));
    }
}

internal void
TriangulateWaterCorner(temporary_hex_mesh * Mesh, hex_cell Cell, hex_direction Direction, v3 p1) {
    if(Direction <= HEX_DIRECTION_E && Cell.Neighbours[Direction] && Cell.Neighbours[Direction+1]) {
        hex_cell Neighbour = *Cell.Neighbours[Direction];
        hex_cell NextNeighbour = *Cell.Neighbours[Direction+1];
        if((Neighbour.WaterLevel <= Neighbour.Elevation) || (NextNeighbour.WaterLevel <= NextNeighbour.Elevation)) return;
        v3 Bridge0 = GetBridgeLocation(Direction);
        v3 Bridge1 = GetBridgeLocation(Direction+1);

        v3 p2 = CrestV3Add(p1, Bridge0);
        v3 p3 = CrestV3Add(p1, Bridge1);

        v3 n0 = NudgeVertex(p1);
        v3 n1 = NudgeVertex(p2);
        v3 n2 = NudgeVertex(p3);
        n0.y = n1.y = n2.y = (Cell.WaterLevel + HEX_WATER_ELEVATION_OFFSET) * HEX_ELEVATION_STEP;


        AddTriangleToHexMeshUnnudged(Mesh, n0, n1, n2);
        AddTriangleColour(Mesh, HEX_WATER_COLOUR);
        AddTriangleUVs(Mesh, v2(0.f, 0.f));
    }
}

internal void
TriangulateShoreCorner(temporary_hex_mesh * Mesh, hex_cell Cell, hex_direction Direction, v3 p1) {
    if(Cell.Neighbours[Direction] && Cell.Neighbours[(Direction+1)%6]) {
        hex_cell Neighbour = *Cell.Neighbours[Direction];
        hex_cell NextNeighbour = *Cell.Neighbours[(Direction+1)%6];

        v3 Bridge0 = GetBridgeLocation(Direction);
        v3 Bridge1 = GetBridgeLocation(Direction+1);
        if(Direction == HEX_DIRECTION_SW) Bridge1 = GetBridgeLocation(0);
        v3 p2 = CrestV3Add(p1, Bridge0);
        v3 p3 = CrestV3Add(p1, Bridge1);

        v3 n0 = NudgeVertex(p1);
        v3 n1 = NudgeVertex(p2);
        v3 n2 = NudgeVertex(p3);
        n0.y = n1.y = n2.y = (Cell.WaterLevel + HEX_WATER_ELEVATION_OFFSET) * HEX_ELEVATION_STEP;


        AddTriangleToHexMeshUnnudged(Mesh, n0, n1, n2);
        AddTriangleColour(Mesh, HEX_WATER_COLOUR);
        AddTriangleUVs3(Mesh,
            v2(0.f, 0.f),
            v2(0.f, 1.f),
            (NextNeighbour.WaterLevel > NextNeighbour.Elevation) ? v2(0.f, 0.f) : v2(0.f, 1.f)
        );
    }
}

internal void
TriangulateShoreConnection(temporary_hex_mesh * Mesh, hex_cell Cell, hex_direction Direction, v3 p0, v3 p1) {
    hex_cell Neighbour = *Cell.Neighbours[Direction];

    v3 Bridge = GetBridgeLocation(Direction);

    v3 p2 = CrestV3Add(p1, Bridge);
    v3 p3 = CrestV3Add(p0, Bridge);

    v3 n0 = NudgeVertex(p0);
    v3 n1 = NudgeVertex(p1);
    v3 n2 = NudgeVertex(p2);
    v3 n3 = NudgeVertex(p3);

    n1.y = n0.y = (Cell.WaterLevel + HEX_WATER_ELEVATION_OFFSET) * HEX_ELEVATION_STEP;
    n2.y = n3.y = n1.y;
    AddQuadToHexMeshUnnudged(Mesh, n1, n0, n3, n2);
    AddQuadColour(Mesh, HEX_WATER_COLOUR);
    AddQuadUvs4(Mesh, v2(0.f, 0.f), v2(0.f, 0.f), v2(0.f, 1.f), v2(0.f, 1.f));
}

internal void
TriangulateWaterCell(temporary_hex_mesh * Mesh, hex_cell Cell) {
    v3 Center = Cell.Position;
    Center.y = (Cell.WaterLevel + HEX_WATER_ELEVATION_OFFSET) * HEX_ELEVATION_STEP;
    v3 Colour = HEX_WATER_COLOUR;
    for(i32 Direction = 0; Direction < 6; ++Direction) {
        i32 NextDirection = (Direction + 1) % 6;
        v3 p0 = CrestV3Add(Center, CrestV3Scale(HexCorners[Direction], HEX_SOLID_FACTOR));
        v3 p1 = CrestV3Add(Center, CrestV3Scale(HexCorners[NextDirection], HEX_SOLID_FACTOR));
        v3 n0 = NudgeVertex(p0);
        v3 n1 = NudgeVertex(p1);

        n1.y = n0.y = p1.y = p0.y = Center.y;

        AddTriangleToHexMeshUnnudged(Mesh, Center, n0, n1);
        AddTriangleColour(Mesh, Colour);
        AddTriangleUVs(Mesh, v2(0.f, 0.f));


        if(Cell.Neighbours[Direction]) {
            if(Cell.Neighbours[Direction]->WaterLevel > Cell.Neighbours[Direction]->Elevation) {
                TriangulateWaterConnection(Mesh, Cell, Direction, p0, p1);
                TriangulateWaterCorner(Mesh, Cell, Direction, p1);
            }
            else {
                TriangulateShoreConnection(Mesh, Cell, Direction, p0, p1);
                TriangulateShoreCorner(Mesh, Cell, Direction, p1);
            }
        }
    }
}

//TODO(Zen): No longer need to send in texture coordinates
internal void
TriangulateWaterMesh(hex_grid * Grid, hex_grid_chunk * Chunk) {
    //this will be the chunk's mesh later on
    hex_mesh * Mesh = &Chunk->WaterMesh;
    temporary_hex_mesh TempMesh = {0};
    for(i32 x = Chunk->X * HEX_CHUNK_WIDTH; x < HEX_CHUNK_WIDTH * (Chunk->X + 1); ++x) {
        for(i32 z = Chunk->Z * HEX_CHUNK_HEIGHT; z < HEX_CHUNK_HEIGHT * (Chunk->Z + 1); ++z) {
            hex_cell Cell = Grid->Cells[z * HEX_CHUNK_WIDTH * HEX_MAX_CHUNKS_HIGH + x];
            if(Cell.WaterLevel > Cell.Elevation) {
                TriangulateWaterCell(&TempMesh, Cell);
            }
        }
    }

    glBindVertexArray(Mesh->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, Mesh->VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, TempMesh.VerticesCount * sizeof(hex_mesh_vertex), TempMesh.Vertices);

    Mesh->VerticesCount = TempMesh.VerticesCount;
}



/*
    Generating Collision Meshes
*/
//TODO(Zen): Nudging vertices means y value is different, probably doesn't matter too much
internal void
TriangulateCollisionCorner(collision_mesh * CollisionMesh, v3 p0, v3 p1, hex_cell Cell, hex_direction NextDirection) {
    if(Cell.Neighbours[NextDirection]) {
        hex_cell NextNeighbour = *Cell.Neighbours[NextDirection];
        v3 Bridge = GetBridgeLocation(NextDirection);
        Bridge.y = NextNeighbour.Position.y - Cell.Position.y;
        v3 p2 = CrestV3Add(p0, Bridge);

        CollisionMesh->Triangles[CollisionMesh->TriangleCount++] = CreateTriangle(
            NudgeVertex(p0), NudgeVertex(p1), NudgeVertex(p2)
        );
    }
}

internal void
TriangulateCollisionConnection(collision_mesh * CollisionMesh, v3 p0, v3 p1, hex_cell Cell, i32 Direction) {
    if(Direction < HEX_DIRECTION_NW) {
        hex_direction NextDirection = Direction + 1;
        if(Cell.Neighbours[Direction]) {
            hex_cell Neighbour = *Cell.Neighbours[Direction];

            v3 Bridge = GetBridgeLocation(Direction);
            Bridge.y = Neighbour.Position.y - Cell.Position.y;
            v3 p2 = CrestV3Add(p1, Bridge);
            v3 p3 = CrestV3Add(p0, Bridge);

            CollisionMesh->Triangles[CollisionMesh->TriangleCount++] = CreateTriangle(
                NudgeVertex(p0), NudgeVertex(p1), NudgeVertex(p2)
            );
            CollisionMesh->Triangles[CollisionMesh->TriangleCount++] = CreateTriangle(
                NudgeVertex(p2), NudgeVertex(p3), NudgeVertex(p0)
            );

            if(Direction < HEX_DIRECTION_NE) {
                TriangulateCollisionCorner(CollisionMesh, p1, p2, Cell, NextDirection);
            }
        }
    }
}

internal void
TriangulateCollisionCell(collision_mesh * CollisionMesh, hex_cell Cell) {
    v3 Center = Cell.Position;
    for(i32 Direction = 0; Direction < 6; ++Direction) {
        i32 NextDirection = (Direction + 1) % 6;
        v3 p0 = CrestV3Add(Center, CrestV3Scale(HexCorners[Direction], HEX_SOLID_FACTOR));
        v3 p1 = CrestV3Add(Center, CrestV3Scale(HexCorners[NextDirection], HEX_SOLID_FACTOR));

        CollisionMesh->Triangles[CollisionMesh->TriangleCount++] = CreateTriangle(
            Center,
            NudgeVertex(p0),
            NudgeVertex(p1)
        );
        TriangulateCollisionConnection(CollisionMesh, p0, p1, Cell, Direction);
    }
}

internal void
CollisionMeshFromChunk(hex_grid * Grid, i32 ChunkIndex) {
    hex_grid_chunk * Chunk = &Grid->Chunks[ChunkIndex];
    collision_mesh * CollisionMesh = &Chunk->CollisionMesh;
    CollisionMesh->TriangleCount = 0;
    for(i32 x = Chunk->X * HEX_CHUNK_WIDTH; x < HEX_CHUNK_WIDTH * (Chunk->X + 1); ++x) {
        for(i32 z = Chunk->Z * HEX_CHUNK_HEIGHT; z < HEX_CHUNK_HEIGHT * (Chunk->Z + 1); ++z) {
            hex_cell Cell = Grid->Cells[z * HEX_CHUNK_WIDTH * HEX_MAX_CHUNKS_HIGH + x];
            TriangulateCollisionCell(CollisionMesh, Cell);
        }
    }

}

internal void
AddLargeCollisionMeshToChunk(hex_grid_chunk * Chunk) {
    r32 MinX = Chunk->X * HEX_CHUNK_WIDTH * HEX_INNER_DIAMETER - HEX_INNER_RADIUS - HEX_NUDGE_STRENGTH;
    r32 MaxX = MinX + HEX_CHUNK_WIDTH * HEX_INNER_DIAMETER + HEX_INNER_RADIUS + 2.f * HEX_NUDGE_STRENGTH;

    r32 MinY = -HEX_ELEVATION_NUDGE_STRENGTH;
    r32 MaxY = (r32)HEX_MAX_ELEVATION * HEX_ELEVATION_STEP + HEX_ELEVATION_STEP;

    r32 MinZ = Chunk->Z * HEX_CHUNK_HEIGHT * HEX_OUTER_DIAMETER - 1.5f * HEX_OUTER_DIAMETER * Chunk->Z - HEX_OUTER_RADIUS * 0.5f - HEX_NUDGE_STRENGTH;
    r32 MaxZ = (Chunk->Z + 1) * HEX_CHUNK_HEIGHT * HEX_OUTER_DIAMETER - 1.5f * HEX_OUTER_DIAMETER * (Chunk->Z + 1) + HEX_NUDGE_STRENGTH + HEX_OUTER_RADIUS;

    //top face
    Chunk->LargeCollisionMesh.Triangles[0] = CreateTriangle(
        v3(MinX, MaxY, MinZ),
        v3(MinX, MaxY, MaxZ),
        v3(MaxX, MaxY, MinZ)
    );
    Chunk->LargeCollisionMesh.Triangles[1] = CreateTriangle(
        v3(MaxX, MaxY, MaxZ),
        v3(MinX, MaxY, MaxZ),
        v3(MaxX, MaxY, MinZ)
    );

    //Back Face
    Chunk->LargeCollisionMesh.Triangles[2] = CreateTriangle(
        v3(MinX, MinY, MinZ),
        v3(MinX, MaxY, MinZ),
        v3(MaxX, MaxY, MinZ)
    );
    Chunk->LargeCollisionMesh.Triangles[3] = CreateTriangle(
        v3(MaxX, MinY, MinZ),
        v3(MinX, MinY, MinZ),
        v3(MaxX, MaxY, MinZ)
    );

    //Front Face
    Chunk->LargeCollisionMesh.Triangles[4] = CreateTriangle(
        v3(MinX, MinY, MaxZ),
        v3(MinX, MaxY, MaxZ),
        v3(MaxX, MaxY, MaxZ)
    );
    Chunk->LargeCollisionMesh.Triangles[5] = CreateTriangle(
        v3(MaxX, MinY, MaxZ),
        v3(MinX, MinY, MaxZ),
        v3(MaxX, MaxY, MaxZ)
    );

    //Left Face
    Chunk->LargeCollisionMesh.Triangles[6] = CreateTriangle(
        v3(MinX, MinY, MinZ),
        v3(MinX, MaxY, MinZ),
        v3(MinX, MaxY, MaxZ)
    );
    Chunk->LargeCollisionMesh.Triangles[7] = CreateTriangle(
        v3(MinX, MinY, MinZ),
        v3(MinX, MinY, MaxZ),
        v3(MinX, MaxY, MaxZ)
    );

    //Right Face
    Chunk->LargeCollisionMesh.Triangles[8] = CreateTriangle(
        v3(MaxX, MinY, MinZ),
        v3(MaxX, MaxY, MinZ),
        v3(MaxX, MaxY, MaxZ)
    );
    Chunk->LargeCollisionMesh.Triangles[9] = CreateTriangle(
        v3(MaxX, MinY, MinZ),
        v3(MaxX, MinY, MaxZ),
        v3(MaxX, MaxY, MaxZ)
    );
}


internal void
ReloadGridVisuals(hex_grid * Grid) {
    //clear cells
    for(i32 x = 0; x < HEX_MAX_CHUNKS_WIDE; ++x) {
        for(i32 z = 0; z < HEX_MAX_CHUNKS_HIGH; ++z) {
            hex_grid_chunk * Chunk = &Grid->Chunks[z * HEX_MAX_CHUNKS_WIDE + x];
            Chunk->X = x;
            Chunk->Z = z;
            InitHexMesh(&Chunk->HexMesh);
            TriangulateMesh(Grid, Chunk);
            InitHexMesh(&Chunk->WaterMesh);
            TriangulateWaterMesh(Grid, Chunk);
            CollisionMeshFromChunk(Grid, z * HEX_MAX_CHUNKS_WIDE + x);
            AddLargeCollisionMeshToChunk(Chunk);
        }
    }
    //Clear mesh data for cells
    for(hex_feature_type Type = 0; Type < HEX_FEATURE_COUNT; ++Type) {
        glBindVertexArray(Grid->FeatureSet.VAOs[Type]);
        glBindBuffer(GL_ARRAY_BUFFER, Grid->FeatureSet.InstancedVBOs[Type]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(matrix) * MAX_FEATURE_SET_SIZE, &Grid->FeatureSet.Features[Type].Model[0]);
    }
}



/*
    Saving and loading
*/
#define BYTES_PER_CELL 6

internal void
SaveGridAsMap(hex_grid * Grid, char * MapName, i32 MapNameLength) {

    //Note(Zen): first 4 bytes are version number

    //Note(Zen): next byte is width then height

    //Note(Zen): Each cell saved as
    // ColourIndex : 1 byte
    // Elevation : 1 byte
    // WaterLevel : 1 byte
    // FeatureDensity : 1 byte
    // FeatureType : 1 byte
    // FeatureDirections : 1 byte

    //6 for header + bytes per cell * number of cells
    char Buffer[4 + 2 + BYTES_PER_CELL * HEX_CELL_COUNT] = {0};
    i32 Cursor = 0;

    //Write Version Num
    Buffer[Cursor++] = 0;
    Buffer[Cursor++] = 0;
    Buffer[Cursor++] = 0;
    Buffer[Cursor++] = 0;

    //Write Width and height
    Buffer[Cursor++] = Grid->Width;
    Buffer[Cursor++] = Grid->Height;

    for(i32 CellIndex = 0; CellIndex < HEX_CELL_COUNT; ++CellIndex) {
        Buffer[Cursor++] = Grid->Cells[CellIndex].ColourIndex;
        Buffer[Cursor++] = Grid->Cells[CellIndex].Elevation;
        Buffer[Cursor++] = Grid->Cells[CellIndex].WaterLevel;
        Buffer[Cursor++] = Grid->Cells[CellIndex].FeatureDensity;
        Buffer[Cursor++] = Grid->Cells[CellIndex].FeatureType;

        i32 Directions = 0;
        for(hex_direction Direction = 0; Direction < HEX_DIRECTION_COUNT; ++Direction) {
            Directions |= Grid->Cells[CellIndex].Features[Direction] << Direction;
        }

        Buffer[Cursor++] = Directions;
    }

    //file path is Maps/<Name>.map
    //since name can be 32 characters long
    char FilePath[5 + 32 + 5] = "Maps/";
    strncat(FilePath, MapName, MapNameLength);
    strncat(FilePath, ".map", 4);
    CrestWriteFile(FilePath, Buffer, Cursor);
}


internal b32
LoadGridFromMap(hex_grid * Grid, char * MapName, i32 MapNameLength) {
    char FilePath[5 + 32 + 5] = "Maps/";
    strncat(FilePath, MapName, MapNameLength);
    strncat(FilePath, ".map", 4);
    char * Buffer = CrestLoadFileAsString(FilePath);

    //TODO(Zen): Log failure
    if(!Buffer) {
        return 0;
    }
    //skip version num
    Grid->Width = Buffer[4];
    Grid->Height = Buffer[5];

    i32 Cursor = 6;
    for(i32 CellIndex = 0; CellIndex < HEX_CELL_COUNT; ++CellIndex) {
        i32 x = CellIndex % HEX_MAX_WIDTH_IN_CELLS;
        i32 z = CellIndex / HEX_MAX_WIDTH_IN_CELLS;

        if(x >= Grid->Width || z >= Grid->Height) {
            Cursor += BYTES_PER_CELL;
            continue;
        }

        Grid->Cells[CellIndex].Index = CellIndex;
        ClearFeaturesFromCell(&Grid->FeatureSet, &Grid->Cells[CellIndex]);

        Grid->Cells[CellIndex].ColourIndex = Buffer[Cursor++];
        Grid->Cells[CellIndex].Elevation = Buffer[Cursor++];
        Grid->Cells[CellIndex].WaterLevel = Buffer[Cursor++];
        Grid->Cells[CellIndex].FeatureDensity = Buffer[Cursor++];
        Grid->Cells[CellIndex].FeatureType = Buffer[Cursor++];

        hex_cell * Cell  = &Grid->Cells[CellIndex];



        Cell->Position = v3((x + z * 0.5f - z/2) * HEX_INNER_DIAMETER, 0.f, z * HEX_OUTER_RADIUS * 1.5f);
        Cell->Position.y = HEX_ELEVATION_STEP * Cell->Elevation;
        v3 Sample = Noise3DSample(Cell->Position);
        Cell->Position.y += Sample.y * HEX_ELEVATION_NUDGE_STRENGTH;


        char Directions = Buffer[Cursor++];
        AddFeaturesToCellMask(&Grid->FeatureSet, Cell, Cell->FeatureType, Directions);

        AddNeighboursToCell(Grid, Cell, x, z);
    }

    free(Buffer);
    return 1; //Successfully loaded
}
