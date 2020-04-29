#define UI_ID_OFFSET 2000

internal void
GameStateInit(app * App) {
    game_state * State = &App->GameState;
    hex_grid * Grid = &App->Grid;
    struct player_info * Player = &State->Player;
    struct enemy_info * Enemy = &State->Enemy;
    State->Camera = CameraInit();


    //TEMP(Zen): Temporary Setup stuff, will eventually be moved to load from
    //scenarios for gameplay (4/20)
    LoadGridFromMap(Grid, "gamestatetest", strlen("gamestatetest"));
    Player->UnitCount = Player->ActiveUnits = 2;
    Player->Units[0].CellIndex = 28 * 4 + 5;
    Player->Units[0].Position = Grid->Cells[Player->Units[0].CellIndex].Position;
    Player->Units[1].CellIndex = 34;
    Player->Units[1].Position = Grid->Cells[Player->Units[1].CellIndex].Position;

    Enemy->UnitCount = 1;
    Enemy->Units[0].CellIndex = 32;
    Enemy->Units[0].Position = Grid->Cells[Enemy->Units[0].CellIndex].Position;

    State->IsPlayerTurn = 1;

    //State->CurrentState = GAME_STATE_PRE_BATTLE (?)
    State->CurrentState = GAME_STATE_NEW_TURN;
    ReloadGridVisuals(Grid);
}

internal void
GameStateUpdate(app * App) {
    camera * Camera = &App->GameState.Camera;
    game_state * GameState = &App->GameState;
    hex_grid * Grid = &App->Grid;
    struct player_info * Player = &GameState->Player;
    struct enemy_info * Enemy = &GameState->Enemy;

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
    if(GameState->IsPlayerTurn) {
        enum game_states NextState = GameState->CurrentState;
        static i32 TurnCount = 0;
        char Buffer[32];
        sprintf(Buffer, "Turn: %d", TurnCount);
        CrestUITextLabelP(&App->UI, GENERIC_ID(0), v4(10, 42, 128, 32), Buffer);

        switch(GameState->CurrentState) {
            case GAME_STATE_NEW_TURN: {
                CrestUITextLabelP(&App->UI, GENERIC_ID(0), v4(10, 10, 128, 32), "Player Turn");
                sprintf(Buffer, "Turn: %.2f", GameState->NewTurn.Time);
                CrestUITextLabelP(&App->UI, GENERIC_ID(0), v4(10, 42 + 32, 128, 32), Buffer);

                GameState->NewTurn.Time += App->Delta;

                if(GameState->NewTurn.Time > NEW_TURN_TIME) {
                    NextState = GAME_STATE_OVERVIEW;
                    GameState->NewTurn.Time = 0.f;
                }
            } break;

            case GAME_STATE_OVERVIEW: {
                CrestUITextLabelP(&App->UI, GENERIC_ID(0), v4(10, 10, 128, 32), "Overview");
                if(AppMouseJustDown(0)) {
                    for(i32 i = 0; i < Player->UnitCount; ++i) {
                        if(HasCollided && (SelectedHexIndex == Player->Units[i].CellIndex)) {
                            Player->SelectedUnit = i;
                            NextState = GAME_STATE_UNIT_SELECTED;
                            Camera->TargetPosition = GetUnitPosition(Grid, Player->Units[Player->SelectedUnit]);
                            GameState->UnitSelected.StartIndex = SelectedHexIndex;
                        }
                    }
                }

                if(AppKeyJustDown(KEY_TAB)) {
                    //TODO(Zen): Skip over exhausted units
                    Player->SelectedUnit += 1;
                    Player->SelectedUnit %= Player->UnitCount;
                    Camera->TargetPosition = GetUnitPosition(Grid, Player->Units[Player->SelectedUnit]);
                }
            } break;

            case GAME_STATE_UNIT_SELECTED: {
                CrestUITextLabelP(&App->UI, GENERIC_ID(0), v4(10, 10, 128, 32), "Unit Selected");
                //Note(Zen): In case selected a different unit while in the state.
                b32 ClearState = 0;
                {
                    unit Unit = Player->Units[Player->SelectedUnit];
                    if(Unit.Exhausted) {
                        NextState = GAME_STATE_OVERVIEW;
                        break;
                    }

                    v2 UIPosition = CrestProjectPoint(Unit.Position, View, Projection,
                                                      App->ScreenWidth, App->ScreenHeight);

                    //TODO(Zen): Figure out a better way to do these actions
                    //so that they aren't as hardcoded(?), as it's going to be a pain
                    //to add more stuff
                    if(!GameState->UnitSelected.HideUI) {
                        CrestUIPushPanel(&App->UI, UIPosition, -0.1f);
                        CrestUIPushRow(&App->UI, UIPosition, v2(64, 32), 1);
                        {
                            if(!Unit.HasMoved) {
                                if(CrestUIButton(&App->UI, GENERIC_ID(0), "Move")) {
                                    GameState->UnitSelected.HideUI = 1;
                                };
                            }

                            if(CrestUIButton(&App->UI, GENERIC_ID(0), "Wait")) {
                                Player->Units[Player->SelectedUnit].Exhausted = 1;
                                NextState = GAME_STATE_OVERVIEW;
                            };
                        }
                        CrestUIPopRow(&App->UI);
                        CrestUIPopPanel(&App->UI);
                    }

                }
                //Note(Zen): See if a different unit chosen
                if(AppMouseJustDown(0) && !App->UI.IsMouseOver) {
                    for(i32 i = 0; i < Player->UnitCount; ++i) {
                        if(HasCollided && (SelectedHexIndex == Player->Units[i].CellIndex)) {
                            Player->SelectedUnit = i;
                            NextState = GAME_STATE_UNIT_SELECTED;
                            ClearState = 1;
                            Camera->TargetPosition = GetUnitPosition(Grid, Player->Units[Player->SelectedUnit]);
                        }
                    }
                }

                //Note(Zen): Show which hexes the selected unit can move to
                if(!Player->Units[Player->SelectedUnit].HasMoved) {
                    i32 CellIndex = Player->Units[Player->SelectedUnit].CellIndex;
                    hex_reachable_cells Accessible = HexGetReachableCells(GameState, Grid, Grid->Cells[CellIndex], 5); //HARDCODE(Zen): The move distance should be given by the unit

                    for(i32 i = 0; i < Accessible.Count; ++i) {
                        if(Accessible.Empty[i]) {
                            C3DDrawCube(&App->Renderer, Grid->Cells[Accessible.Indices[i]].Position,
                                        v3(1.f, 1.f, 0.f), 0.1f);
                            if(!App->UI.IsMouseOver && AppMouseJustDown(0) && (Accessible.Indices[i] == SelectedHexIndex)) {
                                NextState = GAME_STATE_WATCH_MOVE;
                                i32 StartIndex = Player->Units[Player->SelectedUnit].CellIndex;


                                GameState->WatchMove.Path = HexPathingDjikstra(GameState, Grid, Grid->Cells[StartIndex], Grid->Cells[SelectedHexIndex]);
                                GameState->WatchMove.Current = GameState->WatchMove.Path.Count - 1;

                                Player->Units[Player->SelectedUnit].HasMoved = 1;
                                Player->Units[Player->SelectedUnit].CellIndex = Accessible.Indices[i];
                            }
                        }
                    }
                }
                //TODO(Zen): Check that this doesn't let the player exploit multiple moves etc.
                if(AppKeyJustDown(KEY_ESC)) {
                    NextState = GAME_STATE_OVERVIEW;
                    Player->Units[Player->SelectedUnit].CellIndex = GameState->UnitSelected.StartIndex;
                    Player->Units[Player->SelectedUnit].Position = Grid->Cells[Player->Units[Player->SelectedUnit].CellIndex].Position;
                }

                //Note(Zen): Clear info if necessary
                if(NextState != GAME_STATE_UNIT_SELECTED || ClearState) {
                    GameState->UnitSelected.HideUI = 0;
                }
            } break;

            case GAME_STATE_WATCH_MOVE: {
                //TODO(ZEN): (4/20)
                // - Path Smoothing
                // - Rotate To Face Direction
                // - Follow Slopes etc.
                CrestUITextLabelP(&App->UI, GENERIC_ID(0), v4(10, 10, 128, 32), "Watch Move");
                GameState->WatchMove.Time += App->Delta;

                hex_path Path = GameState->WatchMove.Path;

                hex_cell CellA = Grid->Cells[Path.Indices[GameState->WatchMove.Current]];
                hex_cell CellB = Grid->Cells[Path.Indices[GameState->WatchMove.Current - 1]];

                Player->Units[Player->SelectedUnit].Position = CrestV3Lerp(CellA.Position, CellB.Position, GameState->WatchMove.Time * UNIT_SPEED);

                if(GameState->WatchMove.Time * UNIT_SPEED > 1.0f) {
                    GameState->WatchMove.Time -= 1.f / UNIT_SPEED;
                    GameState->WatchMove.Current--;
                }
                if(GameState->WatchMove.Current == 0) {
                    NextState = GAME_STATE_UNIT_SELECTED;
                }

                Camera->TargetPosition = Player->Units[Player->SelectedUnit].Position;
                if(AppKeyJustDown(KEY_ESC)) NextState = GAME_STATE_OVERVIEW;
                //Note(Zen): Clear State Specific Info
                if(NextState != GAME_STATE_WATCH_MOVE) {
                    GameState->WatchMove.Time = 0;
                }
            } break;

            default: {
                Assert(!"Unexpected GameState");
                NextState = GAME_STATE_OVERVIEW;
            } break;
        }
        GameState->CurrentState = NextState;

        //See if need to move to the next turn
        b32 NextTurn = 1;
        for(i32 i = 0; i < Player->UnitCount; ++i) {
            if(!Player->Units[i].Exhausted) NextTurn = 0;
        }
        if(NextTurn) {
            GameState->IsPlayerTurn = 0;
            TurnCount++;

            for(i32 i = 0; i < Player->UnitCount; ++i) {
                Player->Units[i].HasMoved = 0;
                Player->Units[i].Exhausted = 0;
            }
        }
    }
    //Note(Zen): Run the enemy turn
    else {
        GameState->IsPlayerTurn = 1;
        GameState->CurrentState = GAME_STATE_NEW_TURN;
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

    //Terrain
    //TODO(Zen): Frustrum culling.
    for(i32 i = 0; i < HEX_MAX_CHUNKS; ++i) {
        hex_mesh * HexMesh = &Grid->Chunks[i].HexMesh;
        DrawHexMesh(Grid, HexMesh);
    }
    DrawFeatureSet(&Grid->FeatureSet);
    for(i32 i = 0; i < HEX_MAX_CHUNKS; ++i) {
        hex_mesh * WaterMesh = &Grid->Chunks[i].WaterMesh;
        DrawWaterMesh(Grid, WaterMesh);
    }
    //Units
    for(i32 i = 0; i < Player->UnitCount; ++i) {
        v3 Colour = Player->Units[i].Exhausted ? v3(0.f, 0.f, 0.f) : v3(0.3, 0.3, 0.7);
        C3DDrawCube(&App->Renderer, Player->Units[i].Position, Colour, 0.2f);
    }
    for(i32 i = 0; i < Enemy->UnitCount; ++i) {
        v3 Colour = Enemy->Units[i].Exhausted ? v3(0.f, 0.f, 0.f) : v3(1, 0.3, 0.4);
        C3DDrawCube(&App->Renderer, Enemy->Units[i].Position, Colour, 0.2f);
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
