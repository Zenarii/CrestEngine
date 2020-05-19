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
    if(LoadGridFromMap(Grid, "gamestatetest", strlen("gamestatetest"))) {
        strcpy(Grid->MapName, "gamestatetest");
    };
    item Sword = {
        .Name = "Iron Sword",
        .Type = ITEM_TYPE_WEAPON,
        .Damage = 9,
    };
    item Bow = {
        .Name = "Iron Bow",
        .Type = ITEM_TYPE_WEAPON,
        .Damage = 8,
    };

    Player->UnitCount = Player->ActiveUnits = 2;
    Player->Units[0].CellIndex = 28 * 4 + 5;
    Player->Units[0].Position = Grid->Cells[Player->Units[0].CellIndex].Position;
    Player->Units[1].CellIndex = 34;
    Player->Units[1].Position = Grid->Cells[Player->Units[1].CellIndex].Position;

    Player->Inventory[0].Items[0] = Sword; Player->Inventory[0].ItemCount = 1;
    Player->Inventory[1].Items[0] = Sword; Player->Inventory[1].ItemCount = 2;
    Player->Inventory[1].Items[1] = Bow;

    Enemy->UnitCount = 2;
    Enemy->Units[0].CellIndex = 32;
    Enemy->Units[0].Position = Grid->Cells[Enemy->Units[0].CellIndex].Position;
    Enemy->Units[1].CellIndex = 33;
    Enemy->Units[1].Position = Grid->Cells[Enemy->Units[1].CellIndex].Position;

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
        ray_cast RayCast = MakeRaycastFromMouse(Camera, App, View, Projection);

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
                unit Unit = Player->Units[Player->SelectedUnit];
                {
                    if(Unit.Exhausted) {
                        NextState = GAME_STATE_OVERVIEW;
                        break;
                    }
                }

                //Note(Zen): Generate the UI
                i32 CellIndex = Player->Units[Player->SelectedUnit].CellIndex;
                hex_attackable_units Attackable = HexGetAttackableUnits(GameState, Grid, Grid->Cells[CellIndex]);
                //HARDCODE(Zen): unit move distance
                hex_reachable_cells Reachable = HexGetReachableCells(GameState, Grid, Grid->Cells[CellIndex], 5);

                {
                    v2 UIPosition = CrestProjectPoint(Unit.Position, View, Projection,
                                                      App->ScreenWidth, App->ScreenHeight);
                    UIPosition.x += 32;
                    if(!GameState->UnitSelected.HideUI) {
                        CrestUIPushPanel(&App->UI, UIPosition, -0.1f);
                        CrestUIPushRow(&App->UI, UIPosition, v2(100, 32), 1);

                        switch(GameState->UnitSelected.UIState) {
                            case GAME_UI_START: {
                                if(Attackable.Count) {
                                    if(CrestUIButton(&App->UI, GENERIC_ID(0), "Attack")) {
                                        GameState->UnitSelected.UIState = GAME_UI_CHOOSE_TARGET;
                                    };
                                }

                                if(!Unit.HasMoved && Reachable.Count) {
                                    if(CrestUIButton(&App->UI, GENERIC_ID(0), "Move")) {
                                        GameState->UnitSelected.HideUI = 1;
                                    }
                                }

                                if(Player->Inventory[Player->SelectedUnit].ItemCount) {
                                    if(CrestUIButton(&App->UI, GENERIC_ID(0), "Inventory")) {
                                        GameState->UnitSelected.UIState = GAME_UI_INVENTORY;
                                    }
                                }

                                if(CrestUIButton(&App->UI, GENERIC_ID(0), "Wait")) {
                                    Player->Units[Player->SelectedUnit].Exhausted = 1;
                                    NextState = GAME_STATE_OVERVIEW;
                                }
                            } break;

                            case GAME_UI_CHOOSE_TARGET: {
                                char Buffer[32] = {0};
                                sprintf(Buffer, "%d/%d", GameState->UnitSelected.CurrentTarget+1, Attackable.Count);
                                CrestUITextLabel(&App->UI, GENERIC_ID(0), Buffer);
                            } break;

                            case GAME_UI_INVENTORY: {
                                //TODO(Zen): Custom UI widget for inventory items
                                for(i32 i = 0; i < Player->Inventory[Player->SelectedUnit].ItemCount; ++i) {
                                    CrestUIButton(&App->UI, GENERIC_ID(i), Player->Inventory[Player->SelectedUnit].Items[i].Name);
                                }
                                // for(i32 i = Player->Inventory[Player->SelectedUnit].ItemCount; i < MAX_ITEMS; ++i) {
                                //     CrestUITextLabel(&App->UI, GENERIC_ID(0), "");
                                // }

                            } break;
                        }


                        CrestUIPopRow(&App->UI);
                        CrestUIPopPanel(&App->UI);
                    }
                }


                //Note(Zen): See if a different player unit chosen
                if(GameState->UnitSelected.UIState == GAME_UI_START) {
                    b32 ChangedTarget = 0;
                    if(AppMouseJustDown(0) && !App->UI.IsMouseOver) {
                        for(i32 i = 0; i < Player->UnitCount; ++i) {
                            if(HasCollided && (SelectedHexIndex == Player->Units[i].CellIndex)) {
                                Player->SelectedUnit = i;
                                NextState = GAME_STATE_UNIT_SELECTED;
                                ClearState = 1;
                                Camera->TargetPosition = GetUnitPosition(Grid, Player->Units[Player->SelectedUnit]);
                                ChangedTarget = 1;
                            }
                        }
                    }
                    if(!ChangedTarget && AppKeyJustDown(KEY_TAB)) {
                        if(!Player->Units[Player->SelectedUnit].Exhausted && Player->Units[Player->SelectedUnit].HasMoved) {
                            Player->Units[Player->SelectedUnit].HasMoved = 0;
                            Player->Units[Player->SelectedUnit].CellIndex = GameState->UnitSelected.StartIndex;
                            Player->Units[Player->SelectedUnit].Position = GetUnitPosition(Grid, Player->Units[Player->SelectedUnit]);
                        }

                        Player->SelectedUnit += 1;
                        Player->SelectedUnit %= Player->UnitCount;

                        Camera->TargetPosition = Player->Units[Player->SelectedUnit].Position;
                        ChangedTarget = 1;
                    }


                    if(ChangedTarget)
                        GameState->UnitSelected.StartIndex = Player->Units[Player->SelectedUnit].CellIndex;

                }

                //Note(Zen): See if different enemy unit chosen
                else if(GameState->UnitSelected.UIState == GAME_UI_CHOOSE_TARGET) {
                    b32 ChangedTarget = 0;
                    if(AppMouseJustDown(0) && !App->UI.IsMouseOver) {
                        for(i32 i = 0; i < Attackable.Count; ++i) {
                            if(HasCollided && (SelectedHexIndex == Attackable.Indexes[i])) {
                                for (i32 j = 0; j < Enemy->UnitCount; ++j) {
                                    if(SelectedHexIndex == Enemy->Units[j].CellIndex) {
                                        GameState->UnitSelected.CurrentTarget = i;
                                        Camera->TargetPosition = GetUnitPosition(Grid, Player->Units[Player->SelectedUnit]);
                                        ChangedTarget = 1;
                                    }
                                }
                            }
                        }
                    }
                    if(!ChangedTarget && AppKeyJustDown(KEY_TAB)) {
                        GameState->UnitSelected.CurrentTarget += 1;
                        GameState->UnitSelected.CurrentTarget %= Attackable.Count;
                    }
                }



                //Note(Zen): Show which hexes the selected unit can move to and attack
                if(!Player->Units[Player->SelectedUnit].HasMoved) {
                    i32 CellIndex = Player->Units[Player->SelectedUnit].CellIndex;

                    for(i32 i = 0; i < Reachable.Count; ++i) {
                        if(Reachable.Empty[i] && Reachable.Indices[i] != CellIndex) {
                            C3DDrawCube(&App->Renderer, Grid->Cells[Reachable.Indices[i]].Position,
                                        v3(1.f, 1.f, 0.f), 0.15f);
                            if(!App->UI.IsMouseOver && AppMouseJustDown(0) && (Reachable.Indices[i] == SelectedHexIndex) ) {
                                NextState = GAME_STATE_WATCH_MOVE;
                                i32 StartIndex = Player->Units[Player->SelectedUnit].CellIndex;

                                GameState->WatchMove.Path = HexPathingDjikstra(GameState, Grid, Grid->Cells[StartIndex], Grid->Cells[SelectedHexIndex]);
                                GameState->WatchMove.Current = GameState->WatchMove.Path.Count - 1;

                                Player->Units[Player->SelectedUnit].HasMoved = 1;
                                Player->Units[Player->SelectedUnit].CellIndex = Reachable.Indices[i];

                                GameState->WatchMove.StateAfter = GAME_STATE_UNIT_SELECTED;
                                GameState->WatchMove.SubStateAfter = GAME_UI_START;

                            }
                        }
                    }
                    hex_attackable_cells AttackableCells = HexGetAttackableCells(Grid, &Reachable);
                    for(i32 i = 0; i < AttackableCells.Count; ++i) {
                        C3DDrawCube(&App->Renderer, Grid->Cells[AttackableCells.Indices[i]].Position,
                                    v3(1.f, 0.f, 0.f), 0.1f);

                    }
                    if(!App->UI.IsMouseOver && AppMouseJustDown(0)) {
                        for(i32 i = 0; i < AttackableCells.Count; ++i) {
                            if(SelectedHexIndex == AttackableCells.Indices[i]) {
                                for(i32 j = 0; j < Enemy->UnitCount; ++j) {
                                    if(SelectedHexIndex == Enemy->Units[j].CellIndex) {
                                        NextState = GAME_STATE_WATCH_MOVE;
                                        i32 StartIndex = Player->Units[Player->SelectedUnit].CellIndex;


                                        GameState->WatchMove.Path = HexPathingDjikstra(GameState, Grid, Grid->Cells[StartIndex], Grid->Cells[AttackableCells.From[i]]);
                                        GameState->WatchMove.Current = GameState->WatchMove.Path.Count - 1;

                                        Player->Units[Player->SelectedUnit].HasMoved = 1;
                                        Player->Units[Player->SelectedUnit].CellIndex = AttackableCells.From[i];

                                        //TODO(Zen): Maybe choose target unit then watch unit move to attack.
                                        GameState->WatchMove.StateAfter = GAME_STATE_UNIT_SELECTED;
                                        GameState->WatchMove.SubStateAfter = GAME_UI_CHOOSE_TARGET;
                                    }
                                }
                            }
                        }
                    }
                }
                else {
                    i32 CellIndex = Player->Units[Player->SelectedUnit].CellIndex;
                    hex_attackable_cells AttackableCells = HexGetAttackableFromCell(Grid, Grid->Cells[CellIndex]);
                    hex_attackable_units AttackableUnits = HexGetAttackableUnits(GameState, Grid, Grid->Cells[CellIndex]);

                    for(i32 i = 0; i < AttackableCells.Count; ++i) {
                        C3DDrawCube(&App->Renderer, Grid->Cells[AttackableCells.Indices[i]].Position,
                                    v3(1.f, 0.f, 0.f), 0.1f);
                    }
                    if(!App->UI.IsMouseOver && AppMouseJustDown(0)) {
                        for(i32 i = 0; i < AttackableUnits.Count; ++i) {
                            if(SelectedHexIndex == AttackableUnits.Indexes[i]) {
                                GameState->UnitSelected.CurrentTarget = i;
                                GameState->UnitSelected.UIState = GAME_UI_CHOOSE_TARGET;
                            }
                        }
                    }
                }

                if((AppKeyJustDown(KEY_ESC) || App->Mouse.RightDown) && !Player->Units[Player->SelectedUnit].Exhausted) {
                    NextState = GAME_STATE_OVERVIEW;
                    Player->Units[Player->SelectedUnit].CellIndex = GameState->UnitSelected.StartIndex;
                    Player->Units[Player->SelectedUnit].Position = Grid->Cells[Player->Units[Player->SelectedUnit].CellIndex].Position;

                    Player->Units[Player->SelectedUnit].HasMoved = 0;
                }

                //Note(Zen): Clear info if necessary
                if(NextState != GAME_STATE_UNIT_SELECTED || ClearState) {
                    GameState->UnitSelected.HideUI = 0;
                    GameState->UnitSelected.UIState = GAME_UI_START;
                    GameState->UnitSelected.CurrentTarget = 0;
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
                    NextState = GameState->WatchMove.StateAfter;
                }

                Camera->TargetPosition = Player->Units[Player->SelectedUnit].Position;
                //TODO(Zen): Change to the space key
                if(AppKeyJustDown(KEY_P)) {
                    NextState = GAME_STATE_UNIT_SELECTED;
                    Player->Units[Player->SelectedUnit].Position = Grid->Cells[Path.Indices[0]].Position;
                    Camera->TargetPosition = Grid->Cells[Path.Indices[0]].Position;
                }

                if(NextState == GAME_STATE_UNIT_SELECTED) {
                    GameState->UnitSelected.UIState = GameState->WatchMove.SubStateAfter;
                }


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
    EndOfStateSpecific:;
    /*
        Set Uniforms
    */

    {
        v3 CameraLocation = GetCameraLocation(Camera);
        CrestShaderSetMatrix(Grid->MeshShader, "View", &View);
        CrestShaderSetMatrix(Grid->MeshShader, "Model", &Model);
        CrestShaderSetMatrix(Grid->MeshShader, "Projection", &Projection);

        CrestShaderSetV3(Grid->MeshShader, "ViewPosition", CameraLocation);
        CrestShaderSetV3(Grid->MeshShader, "LightColour", v3(1.f, 1.f, 1.f));
        CrestShaderSetV3(Grid->MeshShader, "LightPosition", v3(3.f, 8.f, 3.f));

        CrestShaderSetMatrix(Grid->WaterShader, "View", &View);
        CrestShaderSetMatrix(Grid->WaterShader, "Model", &Model);
        CrestShaderSetMatrix(Grid->WaterShader, "Projection", &Projection);

        CrestShaderSetV3(Grid->WaterShader, "ViewPosition", CameraLocation);
        CrestShaderSetV3(Grid->WaterShader, "LightColour", v3(1.f, 1.f, 1.f));
        CrestShaderSetV3(Grid->WaterShader, "LightPosition", v3(3.f, 8.f, 3.f));
        CrestShaderSetFloat(Grid->WaterShader, "Time", App->TotalTime);

        CrestShaderSetMatrix(Grid->FeatureSet.Shader, "View", &View);
        CrestShaderSetMatrix(Grid->FeatureSet.Shader, "Projection", &Projection);
        CrestShaderSetV3(Grid->FeatureSet.Shader, "Light.Colour", v3(1.f, 1.f, 1.f));
        CrestShaderSetV3(Grid->FeatureSet.Shader, "Light.Position", v3(3.f, 8.f, 3.f));
        CrestShaderSetV3(Grid->FeatureSet.Shader, "ViewPosition", CameraLocation);

    }

    /*
        Draw Everything
    */

    // 1) Draw reflection and refraction textures
    // 2) Bind the main framebuffer and draw (with water shader taking in the new textures)
    // 3) Afterwards draw the UI to a seperate FBO then output to 0
    // 4) Maybe blur behind UI by taking nearby background where depth != starting depth

    glEnable(GL_CLIP_DISTANCE0);

    //Note(Zen): Draw to the refraction texture
    {
        glBindFramebuffer(GL_FRAMEBUFFER, Grid->RefractionFBO.Fbo);
        glClearColor(CLEAR_COLOUR);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        CrestShaderSetV4(Grid->MeshShader, "ClippingPlane", v4(0, -1, 0, HEX_WATER_LEVEL + 0.1f));
        CrestShaderSetMatrix(Grid->MeshShader, "View", &View);
        CrestShaderSetV3(Grid->MeshShader, "ViewPosition", GetCameraLocation(Camera));

        //Note(Zen): Don't draw the feature set as no features can appear underwater right now
        PreDrawHexMesh(Grid);
        for(i32 i = 0; i < HEX_MAX_CHUNKS; ++i) {
            DrawHexMesh(Grid, &Grid->Chunks[i].HexMesh);
        }
    }

    //Note(Zen): Draw to reflection texture
    {
        glBindFramebuffer(GL_FRAMEBUFFER, Grid->ReflectionFBO.Fbo);
        glEnable(GL_DEPTH_TEST);
        glClearColor(CLEAR_COLOUR);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        CrestShaderSetV4(Grid->MeshShader, "ClippingPlane", v4(0, 1, 0, -HEX_WATER_LEVEL + 0.01f));
        CrestShaderSetV4(Grid->FeatureSet.Shader, "ClippingPlane", v4(0, 1, 0, -HEX_WATER_LEVEL + 0.01f));

        camera ReflectionCamera = *Camera;
        ReflectionCamera.Rotation *= -1;
        ReflectionCamera.Position.y += 2.f * (HEX_WATER_LEVEL - ReflectionCamera.Position.y);

        matrix ReflectionViewMatrix = ViewMatrixFromCamera(&ReflectionCamera);
        CrestShaderSetMatrix(Grid->MeshShader, "View", &ReflectionViewMatrix);
        CrestShaderSetV3(Grid->MeshShader, "ViewPosition", GetCameraLocation(&ReflectionCamera));
        CrestShaderSetMatrix(Grid->FeatureSet.Shader, "View", &ReflectionViewMatrix);
        CrestShaderSetV3(Grid->FeatureSet.Shader, "ViewPosition", GetCameraLocation(&ReflectionCamera));

        //Draw reflected meshes
        DrawFeatureSet(&Grid->FeatureSet);
        PreDrawHexMesh(Grid);
        for(i32 i = 0; i < HEX_MAX_CHUNKS; ++i) {
            DrawHexMesh(Grid, &Grid->Chunks[i].HexMesh);
        }
    }

    CrestShaderSetMatrix(Grid->MeshShader, "View", &View);
    CrestShaderSetV3(Grid->MeshShader, "ViewPosition", GetCameraLocation(Camera));
    CrestShaderSetV3(Grid->FeatureSet.Shader, "ViewPosition", GetCameraLocation(Camera));
    CrestShaderSetMatrix(Grid->FeatureSet.Shader, "View", &View);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //Note(Zen): Some drivers ignore disabling the clipping plane
    glDisable(GL_CLIP_DISTANCE0);
    CrestShaderSetV4(Grid->MeshShader, "ClippingPlane", v4(0, 0, 0, 0));
    CrestShaderSetV4(Grid->FeatureSet.Shader, "ClippingPlane", v4(0, 0, 0, 0));

    DrawFeatureSet(&Grid->FeatureSet);
    PreDrawWaterMesh(Grid);
    for(i32 i = 0; i < HEX_MAX_CHUNKS; ++i) {
        DrawWaterMesh(Grid, &Grid->Chunks[i].WaterMesh);
    }
    PreDrawHexMesh(Grid);
    for(i32 i = 0; i < HEX_MAX_CHUNKS; ++i) {
        DrawHexMesh(Grid, &Grid->Chunks[i].HexMesh);
    }


    CrestShaderSetMatrix(App->Renderer.Shader, "View", &View);
    CrestShaderSetMatrix(App->Renderer.Shader, "Model", &Model);
    CrestShaderSetMatrix(App->Renderer.Shader, "Projection", &Projection);

    //Units
    for(i32 i = 0; i < Player->UnitCount; ++i) {
        v3 Colour = Player->Units[i].Exhausted ? v3(0.f, 0.f, 0.f) : v3(0.3, 0.3, 0.7);
        C3DDrawCube(&App->Renderer, Player->Units[i].Position, Colour, 0.2f);
    }
    for(i32 i = 0; i < Enemy->UnitCount; ++i) {
        v3 Colour = Enemy->Units[i].Exhausted ? v3(0.f, 0.f, 0.f) : v3(1, 0.3, 0.4);
        C3DDrawCube(&App->Renderer, Enemy->Units[i].Position, Colour, 0.2f);
    }

    //Note(Zen): Draw the Collision Shapes
    #if 0
    if(GameStateDebug.ShowCollisions) {


        r64 BeforeDrawingColTime = CrestCurrentTime();
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
        r64 AfterDrawingColTime = CrestCurrentTime();
        sprintf(Buffer, "Drawing Collisions: %f", AfterDrawingColTime - BeforeDrawingColTime);
        //CrestUITextLabelP(&App->UI, GENERIC_ID(0), v4(200, 200, 128, 32), Buffer);
    }
    #endif
}

internal void
GameStateFromEditorState(game_state * GameState, editor_state * EditorState) {
    GameState->Camera = EditorState->Camera;
    GameStateDebug.ShowCollisions = EditorStateDebug.ShowCollisions;
}


#undef UI_ID_OFFSET
