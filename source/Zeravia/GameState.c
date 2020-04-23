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

    if(AppKeyJustDown(KEY_F11)) GameStateDebug.ShowCollisions = !GameStateDebug.ShowCollisions;

    matrix IdentityMatrix = {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    };

    doCamera(Camera, App);
    r32 Ratio = App->ScreenWidth / App->ScreenHeight;
    matrix Projection = CrestMatrixPerspective(PI * 0.5f, Ratio, 0.1f, 100.f);
    matrix View = ViewMatrixFromCamera(Camera);
    matrix Model = IdentityMatrix;

    /*
        Main game state loop
    */
    b32 HasCollided = 0;
    hex_coordinates SelectedHex = {0};

    {
        ray_cast RayCast = MakeRaycast(Camera, App, View, Projection);//{RayOrigin, RayDirection};

        r32 OldDistance = 0;
        v3 CollisionPoint = {0};

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
                            hex_coordinates CollidedHex = CartesianToHexCoords(Hit.IntersectionPoint.x, Hit.IntersectionPoint.z);
                            if(HasCollided) {
                                v3 VectorTo = CrestV3Sub(RayCast.Origin, Hit.IntersectionPoint);
                                r32 NewDistance = CrestV3Dot(VectorTo, VectorTo);

                                if(NewDistance < OldDistance) {
                                    NewDistance = OldDistance;
                                    SelectedHex = CollidedHex;
                                    CollisionPoint = Hit.IntersectionPoint;
                                }
                            }
                            else {
                                HasCollided = 1;
                                SelectedHex = CollidedHex;
                                v3 VectorTo = CrestV3Sub(RayCast.Origin, Hit.IntersectionPoint);
                                OldDistance = CrestV3Dot(VectorTo, VectorTo);
                                CollisionPoint = Hit.IntersectionPoint;
                            }
                        }
                    }
                }
            }
        }
        if(HasCollided) {
            i32 CellIndex = GetCellIndex(SelectedHex);
            i32 ColourIndex = Grid->Cells[CellIndex].ColourIndex;
            CrestUITextLabelP(&App->UI, GENERIC_ID(0), v4(16, 16, 128, 32), EditorColourString[ColourIndex]);
            C3DDrawCube(&App->Renderer, CollisionPoint, v3(1.f, 1.f, 1.f), 0.1f);
        }
    } //Collisions

    static b32 UnitSelected = 0;
    static unit Unit = {0};



    C3DDrawCube(&App->Renderer, Grid->Cells[Unit.CellIndex].Position, v3(0.4f, 0.8f, 0.4f), 0.2f);

    if(!UnitSelected) {
        UnitSelected = 1;
    }
    else if(UnitSelected) {
        hex_reachable_cells Accessible = HexGetReachableCells(Grid, Grid->Cells[Unit.CellIndex], 4);

        i32 SelectedCellIndex = GetCellIndex(SelectedHex);
        for(i32 i = 0; i < Accessible.Count; ++i) {
            if(App->Mouse.LeftDown && (SelectedCellIndex == Accessible.Indices[i])) {
                Unit.CellIndex = SelectedCellIndex;
            }
        }


        for(i32 i = 0; i < Accessible.Count; ++i) {
            C3DDrawCube(&App->Renderer, Grid->Cells[Accessible.Indices[i]].Position, v3(0.f, 0.f, 0.2f), 0.1f);
        }

    }


    /*
        Set Uniforms
    */

    {
        CrestShaderSetMatrix(Grid->MeshShader, "View", &View);
        CrestShaderSetMatrix(Grid->MeshShader, "Model", &Model);
        CrestShaderSetMatrix(Grid->MeshShader, "Projection", &Projection);

        CrestShaderSetV3(Grid->MeshShader, "ViewPosition", GetCameraLocation(Camera));
        CrestShaderSetV3(Grid->MeshShader, "LightColour", v3(1.f, 1.f, 1.f));
        CrestShaderSetV3(Grid->MeshShader, "LightPosition", v3(3.f, 8.f, 3.f));

        CrestShaderSetMatrix(Grid->WaterShader, "View", &View);
        CrestShaderSetMatrix(Grid->WaterShader, "Model", &Model);
        CrestShaderSetMatrix(Grid->WaterShader, "Projection", &Projection);

        CrestShaderSetV3(Grid->WaterShader, "ViewPosition", GetCameraLocation(Camera));
        CrestShaderSetV3(Grid->WaterShader, "LightColour", v3(1.f, 1.f, 1.f));
        CrestShaderSetV3(Grid->WaterShader, "LightPosition", v3(3.f, 8.f, 3.f));
        CrestShaderSetFloat(Grid->WaterShader, "Time", App->TotalTime);


        CrestShaderSetMatrix(Grid->FeatureSet.Shader, "View", &View);
        CrestShaderSetMatrix(Grid->FeatureSet.Shader, "Projection", &Projection);
        CrestShaderSetV3(Grid->FeatureSet.Shader, "Light.Colour", v3(1.f, 1.f, 1.f));
        CrestShaderSetV3(Grid->FeatureSet.Shader, "Light.Position", v3(3.f, 8.f, 3.f));
        CrestShaderSetV3(Grid->FeatureSet.Shader, "ViewPosition", GetCameraLocation(Camera));
    }

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




    CrestShaderSetMatrix(App->Renderer.Shader, "View", &View);
    CrestShaderSetMatrix(App->Renderer.Shader, "Model", &Model);
    CrestShaderSetMatrix(App->Renderer.Shader, "Projection", &Projection);

    //Note(Zen): Draw the Collision Shapes
    if(GameStateDebug.ShowCollisions) {
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
    GameStateDebug.ShowCollisions = EditorStateDebug.ShowCollisions;
}


#undef UI_ID_OFFSET
