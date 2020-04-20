#define UI_ID_OFFSET 2000

internal void
GameStateInit(app * App) {
    game_state * State = &App->GameState;
    hex_grid * Grid = &App->Grid;
    State->Camera = CameraInit();

    LoadGridFromMap(Grid, "gamestatetest", strlen("gamestatetest"));

    ReloadGridVisuals(Grid);

}

internal void
GameStateUpdate(app * App) {
    camera * Camera = &App->GameState.Camera;
    game_state * GameState = &App->GameState;
    hex_grid * Grid = &App->Grid;

    matrix IdentityMatrix = {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
    doCamera(Camera, App);

    /*
        Set Uniforms
    */
    r32 Ratio = App->ScreenWidth / App->ScreenHeight;
    matrix Projection = CrestMatrixPerspective(PI * 0.5f, Ratio, 0.1f, 100.f);
    matrix View = ViewMatrixFromCamera(Camera);
    matrix Model = IdentityMatrix;

    CrestShaderSetMatrix(Grid->MeshShader, "View", &View);
    CrestShaderSetMatrix(Grid->MeshShader, "Model", &Model);
    CrestShaderSetMatrix(Grid->MeshShader, "Projection", &Projection);

    CrestShaderSetV3(Grid->MeshShader, "ViewPosition", Camera->Position);
    CrestShaderSetV3(Grid->MeshShader, "LightColour", v3(1.f, 1.f, 1.f));
    CrestShaderSetV3(Grid->MeshShader, "LightPosition", v3(3.f, 8.f, 3.f));

    CrestShaderSetMatrix(Grid->WaterShader, "View", &View);
    CrestShaderSetMatrix(Grid->WaterShader, "Model", &Model);
    CrestShaderSetMatrix(Grid->WaterShader, "Projection", &Projection);

    CrestShaderSetV3(Grid->WaterShader, "ViewPosition", Camera->Position);
    CrestShaderSetV3(Grid->WaterShader, "LightColour", v3(1.f, 1.f, 1.f));
    CrestShaderSetV3(Grid->WaterShader, "LightPosition", v3(3.f, 8.f, 3.f));
    CrestShaderSetFloat(Grid->WaterShader, "Time", App->TotalTime);


    CrestShaderSetMatrix(Grid->FeatureSet.Shader, "View", &View);
    CrestShaderSetMatrix(Grid->FeatureSet.Shader, "Projection", &Projection);
    CrestShaderSetV3(Grid->FeatureSet.Shader, "Light.Colour", v3(1.f, 1.f, 1.f));
    CrestShaderSetV3(Grid->FeatureSet.Shader, "Light.Position", v3(3.f, 8.f, 3.f));
    CrestShaderSetV3(Grid->FeatureSet.Shader, "ViewPosition", Camera->Position);
    /*
        Draw Meshes
    */

    for(i32 i = 0; i < HEX_MAX_CHUNKS; ++i) {
        hex_mesh * HexMesh = &Grid->Chunks[i].HexMesh;
        DrawHexMesh(Grid, HexMesh);
    }
    DrawFeatureSet(&Grid->FeatureSet);
    for(i32 i = 0; i < HEX_MAX_CHUNKS; ++i) {
        hex_mesh * WaterMesh = &Grid->Chunks[i].WaterMesh;
        DrawWaterMesh(Grid, WaterMesh);
    }

    //Note(Zen): Get RayCast
    v4 RayClip = v4(0, 0, -1.f, 1.f);
    RayClip.x = (2.f * App->MousePosition.x) / App->ScreenWidth - 1.f;
    RayClip.y = 1.f - (2.f * App->MousePosition.y) / App->ScreenHeight;

    v4 RayEye = CrestMatrixMultipyV4(CrestMatrixInverse(Projection), RayClip);
    RayEye.z = -1.f;
    RayEye.w = 0.f;
    v4 RayWorld = CrestMatrixMultipyV4(CrestMatrixInverse(View), RayEye);
    v3 RayDirection = CrestV3Normalise(v3(RayWorld.x, RayWorld.y, RayWorld.z));

    v3 RayOrigin = Camera->Position;

    ray_cast RayCast = {RayOrigin, RayDirection};

    static hex_cell StartCell;

    C3DDrawCube(&App->Renderer, StartCell.Position, v3(0.9f, 0.f, 0.2f), 0.1f);

    hex_reachable_cells Reachable = HexGetReachableCells(Grid, StartCell, 5);
    for(i32 i = 0; i < Reachable.Count; ++i) {
        C3DDrawCube(&App->Renderer,
                    Grid->Cells[Reachable.Indices[i]].Position,
                    CrestV3Sub(v3(1.f, 1.f, 1.f), HexColours[Grid->Cells[Reachable.Indices[i]].ColourIndex]), 0.1f);
    }

    for(i32 z = 0; z < HEX_MAX_CHUNKS_HIGH; ++z) {
        for(i32 x = 0; x < HEX_MAX_CHUNKS_WIDE; ++x) {
            i32 Index = z * HEX_MAX_CHUNKS_WIDE + x;
            hex_grid_chunk * Chunk = &Grid->Chunks[Index];

            b32 HitChunk = CheckRayThroughChunk(Chunk, RayCast);
            if(HitChunk) {
                for(i32 TriIndex = 0; TriIndex < Chunk->CollisionMesh.TriangleCount; ++TriIndex) {
                    collision_triangle Triangle = Chunk->CollisionMesh.Triangles[TriIndex];
                    collision_result Hit = RayTriangleIntersect(RayCast.Origin, RayCast.Direction, &Triangle);

                    if(Hit.DidIntersect) {
                        hex_coordinates SelectedHex = CartesianToHexCoords(Hit.IntersectionPoint.x, Hit.IntersectionPoint.z);
                        i32 CellIndex = GetCellIndex(SelectedHex);
                        i32 ColourIndex = Grid->Cells[CellIndex].ColourIndex;
                        CrestUITextLabelP(&App->UI, GENERIC_ID(0), v4(16, 16, 128, 32), EditorColourString[ColourIndex]);
                        C3DDrawCube(&App->Renderer, Hit.IntersectionPoint, v3(1.f, 1.f, 1.f), 0.1f);

                        if(App->LeftMouseDown) {
                            StartCell = Grid->Cells[CellIndex];
                        }

                        goto EditStateEndOfCollisions;

                    }
                }
            }
        }
    }
    EditStateEndOfCollisions:;





    CrestShaderSetMatrix(App->Renderer.Shader, "View", &View);
    CrestShaderSetMatrix(App->Renderer.Shader, "Model", &Model);
    CrestShaderSetMatrix(App->Renderer.Shader, "Projection", &Projection);

    //Note(Zen): Draw the Collision Shapes
    if(App->KeyDown[KEY_P]) {
        C3DFlush(&App->Renderer);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        for(i32 x = 0; x < HEX_MAX_CHUNKS_WIDE; ++x) {
            for(i32 z = 0; z < HEX_MAX_CHUNKS_HIGH; ++z) {
                collision_mesh * CollisionMesh = &Grid->Chunks[z * HEX_MAX_CHUNKS_WIDE + x].CollisionMesh;
                for(i32 TriIndex = 0; TriIndex < CollisionMesh->TriangleCount; ++TriIndex) {
                    collision_triangle Triangle = CollisionMesh->Triangles[TriIndex];
                    C3DDrawTri(&App->Renderer, Triangle.Vertex0, Triangle.Vertex1, Triangle.Vertex2, v3(1.f, 0.f, 0.f));
                }
            }
        }
        C3DFlush(&App->Renderer);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

internal void
GameStateFromEditorState(game_state * GameState, editor_state * EditorState) {
    GameState->Camera = EditorState->Camera;
}


#undef UI_ID_OFFSET
