#define UI_ID_OFFSET 2000

internal void
GameStateInit(app * App) {
    game_state * State = &App->GameState;
    hex_grid * Grid = &App->Grid;
    State->Camera = CameraInit();


    //TEMP(Zen): Temporary Setup stuff, will eventually be moved to load from
    //scenarios for gameplay (4/20)
    LoadGridFromMap(Grid, "gamestatetest", strlen("gamestatetest"));
    State->PlayerUnitsCount = State->ActiveUnits = 2;
    State->PlayerUnits[0].CellIndex = 28 * 4 + 5;
    State->PlayerUnits[0].Position = Grid->Cells[State->PlayerUnits[0].CellIndex].Position;
    State->PlayerUnits[1].CellIndex = 34;
    State->PlayerUnits[1].Position = Grid->Cells[State->PlayerUnits[1].CellIndex].Position;
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
    i32 SelectedHexIndex = 0;
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
                                    OldDistance = NewDistance;
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
        //TODO(Zen): Remove this when necessary
        if(HasCollided) {
            C3DDrawCube(&App->Renderer, CollisionPoint, v3(1.f, 1.f, 1.f), 0.1f);
        }
        SelectedHexIndex = GetCellIndex(SelectedHex);
    }

    //Note(Zen): Game Logic
    {
        enum game_states NextState = GameState->CurrentState;

        switch(GameState->CurrentState) {
            case GAME_STATE_OVERVIEW: {
                CrestUITextLabelP(&App->UI, GENERIC_ID(0), v4(10, 10, 128, 32), "Overview");
                if(AppMouseJustDown(0)) {
                    for(i32 i = 0; i < GameState->PlayerUnitsCount; ++i) {
                        if(HasCollided && (SelectedHexIndex == GameState->PlayerUnits[i].CellIndex) && !GameState->PlayerUnits[i].Exhausted) {
                            GameState->SelectedUnit = i;
                            NextState = GAME_STATE_UNIT_SELECTED;
                            Camera->TargetPosition = GetUnitPosition(Grid, GameState->PlayerUnits[GameState->SelectedUnit]);
                        }
                    }
                }

                if(AppKeyJustDown(KEY_TAB)) {
                    //TODO(Zen): Skip over exhausted units
                    GameState->SelectedUnit += 1;
                    GameState->SelectedUnit %= GameState->PlayerUnitsCount;
                    Camera->TargetPosition = GetUnitPosition(Grid, GameState->PlayerUnits[GameState->SelectedUnit]);
                }
            } break;

            case GAME_STATE_UNIT_SELECTED: {
                CrestUITextLabelP(&App->UI, GENERIC_ID(0), v4(10, 10, 128, 32), "UnitSelected");

                {
                    unit Unit = GameState->PlayerUnits[GameState->SelectedUnit];
                    v2 UIPosition = CrestProjectPoint(Unit.Position, View, Projection,
                                                      App->ScreenWidth, App->ScreenHeight);

                    CrestUIPushPanel(&App->UI, UIPosition, -0.1f);
                    CrestUIPushRow(&App->UI, UIPosition, v2(64, 32), 1);
                    {
                        CrestUIButton(&App->UI, GENERIC_ID(0), "Move");
                        if(CrestUIButton(&App->UI, GENERIC_ID(0), "Wait")) {
                            GameState->PlayerUnits[GameState->SelectedUnit].Exhausted = 1;
                            NextState = GAME_STATE_OVERVIEW;
                        };
                    }
                    CrestUIPopRow(&App->UI);
                    CrestUIPopPanel(&App->UI);


                }
                //Note(Zen): See if a different unit chosen
                if(AppMouseJustDown(0) && !App->UI.IsMouseOver) {
                    for(i32 i = 0; i < GameState->PlayerUnitsCount; ++i) {
                        if(HasCollided && (SelectedHexIndex == GameState->PlayerUnits[i].CellIndex)) {
                            GameState->SelectedUnit = i;
                            NextState = GAME_STATE_UNIT_SELECTED;
                            Camera->TargetPosition = GetUnitPosition(Grid, GameState->PlayerUnits[GameState->SelectedUnit]);
                        }
                    }
                }

                //Note(Zen): Show which hexes the selected unit can move to
                if(!GameState->PlayerUnits[GameState->SelectedUnit].HasMoved) {
                    i32 CellIndex = GameState->PlayerUnits[GameState->SelectedUnit].CellIndex;
                    hex_reachable_cells Accessible = HexGetReachableCells(Grid, Grid->Cells[CellIndex], 5); //HARDCODE(Zen): The move distance should be given by the unit

                    for(i32 i = 0; i < Accessible.Count; ++i) {
                        C3DDrawCube(&App->Renderer, Grid->Cells[Accessible.Indices[i]].Position,
                                    v3(1.f, 1.f, 0.f), 0.1f);
                        if(!App->UI.IsMouseOver && AppMouseJustDown(0) && (Accessible.Indices[i] == SelectedHexIndex)) {
                            NextState = GAME_STATE_WATCH_MOVE;
                            i32 StartIndex = GameState->PlayerUnits[GameState->SelectedUnit].CellIndex;


                            GameState->WatchMove.Path = HexPathingDjikstra(Grid, Grid->Cells[StartIndex], Grid->Cells[SelectedHexIndex]);
                            GameState->WatchMove.Current = GameState->WatchMove.Path.Count - 1;

                            GameState->PlayerUnits[GameState->SelectedUnit].HasMoved = 1;
                            GameState->PlayerUnits[GameState->SelectedUnit].CellIndex = Accessible.Indices[i];
                        }
                    }
                }

                if(AppKeyJustDown(KEY_ESC)) NextState = GAME_STATE_OVERVIEW;
            } break;

            case GAME_STATE_WATCH_MOVE: {
                //TODO(ZEN): (4/20)
                // - Path Smoothing
                // - Rotate To Face Direction
                // - Follow Slopes etc.
                CrestUITextLabelP(&App->UI, GENERIC_ID(0), v4(10, 10, 128, 32), "Watch Move");

                hex_path Path = GameState->WatchMove.Path;

                static r32 Time = 0;
                Time += App->Delta;
                char Buffer [32] = {0};
                sprintf(Buffer, "%.2f", Time * UNIT_SPEED);
                CrestUITextLabelP(&App->UI, GENERIC_ID(0), v4(10, 42, 128, 32), Buffer);

                hex_cell CellA = Grid->Cells[Path.Indices[GameState->WatchMove.Current]];
                hex_cell CellB = Grid->Cells[Path.Indices[GameState->WatchMove.Current - 1]];

                GameState->PlayerUnits[GameState->SelectedUnit].Position = CrestV3Lerp(CellA.Position, CellB.Position, Time * UNIT_SPEED);

                if(Time * UNIT_SPEED > 1.0f) {
                    Time -= 1.f / UNIT_SPEED;
                    GameState->WatchMove.Current--;
                }
                if(GameState->WatchMove.Current == 0) {
                    NextState = GAME_STATE_UNIT_SELECTED;
                }

                Camera->TargetPosition = GameState->PlayerUnits[GameState->SelectedUnit].Position;
                if(AppKeyJustDown(KEY_ESC)) NextState = GAME_STATE_OVERVIEW;
            } break;

            default: {
                Assert(!"Unexpected GameState");
                NextState = GAME_STATE_OVERVIEW;
            } break;
        }
        GameState->CurrentState = NextState;
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
        Draw Everything
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
    for(i32 i = 0; i < GameState->PlayerUnitsCount; ++i) {
        v3 Colour = GameState->PlayerUnits[i].Exhausted ? v3(0.f, 0.f, 0.f) : v3(0.3, 0.3, 0.7);
        C3DDrawCube(&App->Renderer, GameState->PlayerUnits[i].Position, Colour, 0.2f);
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
