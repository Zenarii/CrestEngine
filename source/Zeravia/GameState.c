#define UI_ID_OFFSET 2000

internal void
GameStateInit(app * App) {
    game_state * State = &App->GameState;
    State->Camera = CameraInit();

    hex_grid * Grid = &App->GameState.Grid;
    {
        Grid->MeshShader = CrestShaderInit("../assets/hex_shader.vs", "../assets/hex_shader.fs");
        glUseProgram(Grid->MeshShader);
        i32 Location = glGetUniformLocation(Grid->MeshShader, "Images");
        int samplers[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
        glUniform1iv(Location, 16, samplers);

        Grid->MeshTexture = CasLoadTexture("../assets/White.png", GL_LINEAR);

        Grid->WaterShader = CrestShaderInit("../assets/hex_water_shader.vs", "../assets/hex_water_shader.fs");
        glUseProgram(Grid->WaterShader);
        Location = glGetUniformLocation(Grid->WaterShader, "Images");
        glUniform1iv(Location, 16, samplers);
        Grid->WaterTexture = CasLoadTexture("../assets/NoiseTexture.png", GL_LINEAR);
    }
    Grid->Width = HEX_MAX_WIDTH_IN_CELLS;
    Grid->Height = HEX_CHUNK_HEIGHT * HEX_MAX_CHUNKS_HIGH;

    InitFeatureSet(&State->Grid.FeatureSet);

    LoadGridFromMap(Grid, "gamestatetest", strlen("gamestatetest"));

    ReloadGridVisuals(Grid);

}

internal void
GameStateUpdate(app * App) {
    camera * Camera = &App->GameState.Camera;
    game_state * GameState = &App->GameState;

    matrix IdentityMatrix = {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
    if(App->KeyDown[KEY_W]) Camera->Position.z -= CAMERA_SPEED * App->Delta;
    if(App->KeyDown[KEY_S]) Camera->Position.z += CAMERA_SPEED * App->Delta;
    if(App->KeyDown[KEY_A]) Camera->Position.x -= CAMERA_SPEED * App->Delta;
    if(App->KeyDown[KEY_D]) Camera->Position.x += CAMERA_SPEED * App->Delta;
    if(App->KeyDown[KEY_Q]) Camera->Rotation   += 3 * App->Delta;
    if(App->KeyDown[KEY_E]) Camera->Rotation   -= 3 * App->Delta;

    if(Camera->Rotation > PI * 0.5f) Camera->Rotation = PI * 0.5f;
    if(Camera->Rotation < PI * 0.1f) Camera->Rotation = PI * 0.1f;

    /*
        Set Uniforms
    */
    r32 Ratio = App->ScreenWidth / App->ScreenHeight;
    matrix Projection = CrestMatrixPerspective(PI * 0.5f, Ratio, 0.1f, 100.f);
    matrix View = ViewMatrixFromCamera(Camera);
    matrix Model = IdentityMatrix;

    CrestShaderSetMatrix(GameState->Grid.MeshShader, "View", &View);
    CrestShaderSetMatrix(GameState->Grid.MeshShader, "Model", &Model);
    CrestShaderSetMatrix(GameState->Grid.MeshShader, "Projection", &Projection);

    CrestShaderSetV3(GameState->Grid.MeshShader, "ViewPosition", Camera->Position);
    CrestShaderSetV3(GameState->Grid.MeshShader, "LightColour", v3(1.f, 1.f, 1.f));
    CrestShaderSetV3(GameState->Grid.MeshShader, "LightPosition", v3(3.f, 8.f, 3.f));

    CrestShaderSetMatrix(GameState->Grid.WaterShader, "View", &View);
    CrestShaderSetMatrix(GameState->Grid.WaterShader, "Model", &Model);
    CrestShaderSetMatrix(GameState->Grid.WaterShader, "Projection", &Projection);

    CrestShaderSetV3(GameState->Grid.WaterShader, "ViewPosition", Camera->Position);
    CrestShaderSetV3(GameState->Grid.WaterShader, "LightColour", v3(1.f, 1.f, 1.f));
    CrestShaderSetV3(GameState->Grid.WaterShader, "LightPosition", v3(3.f, 8.f, 3.f));
    CrestShaderSetFloat(GameState->Grid.WaterShader, "Time", App->TotalTime);


    CrestShaderSetMatrix(GameState->Grid.FeatureSet.Shader, "View", &View);
    CrestShaderSetMatrix(GameState->Grid.FeatureSet.Shader, "Projection", &Projection);
    CrestShaderSetV3(GameState->Grid.FeatureSet.Shader, "Light.Colour", v3(1.f, 1.f, 1.f));
    CrestShaderSetV3(GameState->Grid.FeatureSet.Shader, "Light.Position", v3(3.f, 8.f, 3.f));
    CrestShaderSetV3(GameState->Grid.FeatureSet.Shader, "ViewPosition", Camera->Position);
    /*
        Draw Meshes
    */

    for(i32 i = 0; i < HEX_MAX_CHUNKS; ++i) {
        hex_mesh * HexMesh = &GameState->Grid.Chunks[i].HexMesh;
        DrawHexMesh(&GameState->Grid, HexMesh);
    }
    DrawFeatureSet(&GameState->Grid.FeatureSet);
    for(i32 i = 0; i < HEX_MAX_CHUNKS; ++i) {
        hex_mesh * WaterMesh = &GameState->Grid.Chunks[i].WaterMesh;
        DrawWaterMesh(&GameState->Grid, WaterMesh);
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


    for(i32 z = 0; z < HEX_MAX_CHUNKS_HIGH; ++z) {
        for(i32 x = 0; x < HEX_MAX_CHUNKS_WIDE; ++x) {
            i32 Index = z * HEX_MAX_CHUNKS_WIDE + x;
            hex_grid_chunk * Chunk = &GameState->Grid.Chunks[Index];

            b32 HitChunk = CheckRayThroughChunk(Chunk, RayCast);
            if(HitChunk) {
                for(i32 TriIndex = 0; TriIndex < Chunk->CollisionMesh.TriangleCount; ++TriIndex) {
                    collision_triangle Triangle = Chunk->CollisionMesh.Triangles[TriIndex];
                    collision_result Hit = RayTriangleIntersect(RayCast.Origin, RayCast.Direction, &Triangle);

                    if(Hit.DidIntersect) {
                        hex_coordinates SelectedHex = CartesianToHexCoords(Hit.IntersectionPoint.x, Hit.IntersectionPoint.z);
                        i32 CellIndex = GetCellIndex(SelectedHex);
                        i32 ColourIndex = GameState->Grid.Cells[CellIndex].ColourIndex;
                        CrestUITextLabelP(&App->UI, GENERIC_ID(0), v4(16, 16, 128, 32), EditorColourString[ColourIndex]);
                        C3DDrawCube(&App->Renderer, Hit.IntersectionPoint, v3(1.f, 1.f, 1.f), 0.1f);
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
                collision_mesh * CollisionMesh = &GameState->Grid.Chunks[z * HEX_MAX_CHUNKS_WIDE + x].CollisionMesh;
                for(i32 TriIndex = 0; TriIndex < CollisionMesh->TriangleCount; ++TriIndex) {
                    collision_triangle Triangle = CollisionMesh->Triangles[TriIndex];
                    C3DDrawTri(&App->Renderer, Triangle.Vertex0, Triangle.Vertex1, Triangle.Vertex2, v3(1.f, 0.f, 0.f));
                }
            }
        }
        C3DFlush(&App->Renderer);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

#if 0
    if(EditorStateDebug.ShowLargeCollisions) {
        C3DFlush(&App->Renderer);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        for(i32 x = 0; x < HEX_MAX_CHUNKS_WIDE; ++x) {
            for(i32 z = 0; z < HEX_MAX_CHUNKS_HIGH; ++z) {
                for(i32 i = 0; i < 10; ++i) {
                    large_collision_mesh * LargeCollisionMesh = &App->EditorState.HexGrid.Chunks[z * HEX_MAX_CHUNKS_WIDE + x].LargeCollisionMesh;

                    collision_triangle Triangle = LargeCollisionMesh->Triangles[i];
                    C3DDrawTri(&App->Renderer, Triangle.Vertex0, Triangle.Vertex1, Triangle.Vertex2, v3(0.f, 0.9f, 1.f));
                }
            }
        }
        C3DFlush(&App->Renderer);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
#endif
}
#undef UI_ID_OFFSET
