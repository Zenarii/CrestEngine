#define UI_ID_OFFSET 1000

internal void
EditorStateInit(app * App) {
    editor_state * State = &App->EditorState;
    State->Camera = CameraInit();


    hex_grid * Grid = &App->Grid;

    if(!CrestDoesFileExist("Maps")) {
        CrestMakeDirectory("Maps");
    }

    ResetCellsOnHexGrid(Grid);
    ReloadGridVisuals(Grid);

    EditorStateDebug.ShowUI = 1;
}


internal hex_edit_settings
doEditorUITerrain(CrestUI * ui, hex_edit_settings Settings) {
    Settings.EditColour = CrestUIToggleButton(ui, GENERIC_ID(0), Settings.EditColour, "Colour:");
    if(Settings.EditColour) {
        #define HexColour(name, upper_name, r, g, b) Settings.ColourIndex = \
                                CrestUIToggleButton(ui, GENERIC_ID(EDITOR_COLOUR_##upper_name), \
                                Settings.ColourIndex == EDITOR_COLOUR_##upper_name, #name) \
                                ? EDITOR_COLOUR_##upper_name : Settings.ColourIndex;
        #include "HexColours.inc"
        CrestUITextLabel(ui, GENERIC_ID(0), "");
    }
    /*
        Elevation
    */
    Settings.EditElevation = CrestUIToggleButton(ui, GENERIC_ID(0), Settings.EditElevation, "Elevation:");
    if(Settings.EditElevation) {
        Settings.Elevation = CrestUISliderInt(ui, GENERIC_ID(0), Settings.Elevation, HEX_MAX_ELEVATION, "Elevation");
    }
    /*
        Water Editing
    */
    Settings.EditWater = CrestUIToggleButton(ui, GENERIC_ID(0), Settings.EditWater, "Water:");
    if(Settings.EditWater) Settings.WaterLevel = CrestUISliderInt(ui, GENERIC_ID(0), Settings.WaterLevel, HEX_MAX_ELEVATION, "Water Level");


    Settings.BrushSize = 0;//CrestUISliderInt(ui, GENERIC_ID(0), Settings.BrushSize, 4, "BrushSize");

    return Settings;
}

internal hex_edit_settings
doEditorUITerrainFeatures(CrestUI * ui, hex_edit_settings Settings) {
    #define HexFeature(Name) Settings.EditFeature = CrestUIToggleButton(ui, GENERIC_ID(HEX_FEATURE_##Name), Settings.EditFeature == HEX_FEATURE_##Name, #Name "s:") ? HEX_FEATURE_##Name : Settings.EditFeature;
    #include "HexGrid/HexFeatures.inc"


    Settings.FeatureDensity = CrestUISliderInt(ui, GENERIC_ID(0), Settings.FeatureDensity, 6, "Density");

    return Settings;
}

internal hex_edit_settings
doEditorUI(CrestUI * ui, hex_edit_settings Settings, r32 ScreenWidth) {
    CrestUIPushPanel(ui, v2(1.f, 1.f), -0.1f);
    CrestUIPushRow(ui, v2(1.f, 1.f), v2(150, 32.f), EDIT_MODE_COUNT);
    {
        Settings.EditMode = CrestUIButton(ui, GENERIC_ID(0), "Terrain") ? EDIT_MODE_TERRAIN : Settings.EditMode;
        Settings.EditMode = CrestUIButton(ui, GENERIC_ID(0), "Features") ? EDIT_MODE_TERRAIN_FEATURES : Settings.EditMode;

        Settings.EditMode = CrestUIButton(ui, GENERIC_ID(0), "Save Map") ? EDIT_MODE_SAVING : Settings.EditMode;
        Settings.EditMode = CrestUIButton(ui, GENERIC_ID(0), "Load Map") ? EDIT_MODE_LOADING : Settings.EditMode;
        Settings.EditMode = CrestUIButton(ui, GENERIC_ID(0), "New Map") ? EDIT_MODE_NEW_MAP : Settings.EditMode;


    }
    CrestUIPopRow(ui);
    CrestUIPopPanel(ui);

    if(Settings.EditMode < EDIT_MODE_SAVING) {
        CrestUIPushPanel(ui, v2(10.f, 47.f), -0.1f);
        CrestUIPushRow(ui, v2(10.f, 47.f), v2(150, 32), 1);

        if(Settings.EditMode == EDIT_MODE_TERRAIN) Settings = doEditorUITerrain(ui, Settings);
        else if (Settings.EditMode == EDIT_MODE_TERRAIN_FEATURES) Settings = doEditorUITerrainFeatures(ui, Settings);

        CrestUIPopRow(ui);
        CrestUIPopPanel(ui);
    }
    return Settings;
}

internal b32
EditCellTerrain(hex_cell * Cell, hex_edit_settings Settings) {
    b32 Result = 0;
    if(Settings.EditColour && Cell->ColourIndex != Settings.ColourIndex) {
        Cell->ColourIndex = Settings.ColourIndex;
        Result |= EDITSTATE_EDITED_MESH;
    }
    if(Settings.EditElevation && (Cell->Elevation != Settings.Elevation)) {
        Cell->Elevation = Settings.Elevation;
        Cell->Position.y = Cell->Elevation * HEX_ELEVATION_STEP;
        v3 Sample = Noise3DSample(Cell->Position);
        Cell->Position.y += Sample.y * HEX_ELEVATION_NUDGE_STRENGTH;
        Result |= (EDITSTATE_EDITED_COLLISIONS | EDITSTATE_EDITED_MESH
                   | EDITSTATE_EDITED_WATER | EDITSTATE_EDITED_FEATURES);
    }
    if(Settings.EditWater && Settings.WaterLevel != Cell->WaterLevel) {
        Cell->WaterLevel = Settings.WaterLevel;
        Result |= EDITSTATE_EDITED_WATER;
    }
    return Result;
}

internal b32
EditCellTerrainFeatures(hex_cell * Cell, hex_feature_set * FeatureSet, hex_edit_settings Settings) {
    b32 Result = 0;

    if(Settings.EditFeature) {
        if((Settings.FeatureDensity != Cell->FeatureDensity) || (Settings.EditFeature != Cell->FeatureType)) {
            ClearFeaturesFromCell(FeatureSet, Cell);
            AddFeaturesToCell(FeatureSet, Cell, Settings.EditFeature, Settings.FeatureDensity);
        }
    }
    else {
        ClearFeaturesFromCell(FeatureSet, Cell);
    }

    return Result;
}



internal b32
EditCell(hex_cell * Cell, hex_feature_set * FeatureSet, hex_edit_settings Settings) {
    b32 Result = 0;
    if(Settings.EditMode == EDIT_MODE_TERRAIN) Result |= EditCellTerrain(Cell, Settings);
    else if(Settings.EditMode == EDIT_MODE_TERRAIN_FEATURES) Result |= EditCellTerrainFeatures(Cell, FeatureSet, Settings);
    return Result;
}

/* TODO(Zen): Fix BrushSize
internal b32
EditCells(hex_grid * Grid, i32 StartCellIndex, hex_edit_settings Settings) {
    hex_cell * StartCell = &Grid->Cells[StartCellIndex];
    i32 CenterX = StartCellIndex % HEX_MAX_WIDTH_IN_CELLS;
    i32 CenterZ = StartCellIndex / HEX_MAX_WIDTH_IN_CELLS;

    b32 EditedACell = 0;

    for(i32 r = 0, z = CenterZ - Settings.BrushSize; z <= CenterZ; ++z, ++r) {
        for(i32 x = CenterX - r; x <= CenterX + Settings.BrushSize; ++x) {
            i32 CellIndex = z * HEX_MAX_WIDTH_IN_CELLS + x;
            EditedACell = EditCell(&Grid->Cells[CellIndex], Settings) ? 1 : EditedACell;
        }
    }

    for(i32 r = 0, z = CenterZ + Settings.BrushSize; z > CenterZ; --z, ++r) {
        for(i32 x = CenterX - Settings.BrushSize; x <= CenterX + r; ++x) {
            i32 CellIndex = z * HEX_MAX_WIDTH_IN_CELLS + x;
            EditedACell = EditCell(&Grid->Cells[CellIndex], Settings) ? 1 : EditedACell;
        }
    }

    return EditedACell;
}
*/
internal b32
EditorCheckCollisionsOnChunk(i32 ChunkIndex, hex_grid * Grid, hex_edit_settings Settings, ray_cast RayCast) {
    b32 CollidedWithThisChunk = 0;
    hex_grid_chunk * Chunk = &Grid->Chunks[ChunkIndex];

    for(i32 TriIndex = 0; TriIndex < Chunk->CollisionMesh.TriangleCount; ++TriIndex) {
        collision_triangle Triangle = Chunk->CollisionMesh.Triangles[TriIndex];
        collision_result Hit = RayTriangleIntersect(RayCast.Origin, RayCast.Direction, &Triangle);

        if(Hit.DidIntersect) {
            CollidedWithThisChunk = 1;
            hex_coordinates SelectedHex = CartesianToHexCoords(Hit.IntersectionPoint.x, Hit.IntersectionPoint.z);
            i32 CellIndex = GetCellIndex(SelectedHex);
            //change the colour
            if(CellIndex > -1) {
                b32 Result = EditCell(&Grid->Cells[CellIndex], &Grid->FeatureSet, Settings);
                if(Result & EDITSTATE_EDITED_MESH) {
                    TriangulateMesh(Grid, Chunk);
                    i32 PreviousChunkIndex = -1;
                    //Note(Zen): Edit any neighbouring chunks that require it
                    for(i32 i = 0; i < 6; ++i) {
                        if(Grid->Cells[CellIndex].Neighbours[i]) {
                            hex_cell Neighbour = *Grid->Cells[CellIndex].Neighbours[i];
                            i32 NeighbourChunkIndex = GetChunkIndexFromCellIndex(Neighbour.Index);
                            if(NeighbourChunkIndex != ChunkIndex && NeighbourChunkIndex != PreviousChunkIndex) {
                                TriangulateMesh(Grid, &Grid->Chunks[NeighbourChunkIndex]);
                                PreviousChunkIndex = NeighbourChunkIndex;
                            }
                        }
                    }
                }
                if(Result & EDITSTATE_EDITED_COLLISIONS) {
                    CollisionMeshFromChunk(Grid, ChunkIndex);
                    i32 PreviousChunkIndex = -1;
                    for(i32 i = 0; i < 6; ++i) {
                        if(Grid->Cells[CellIndex].Neighbours[i]) {
                            hex_cell Neighbour = *Grid->Cells[CellIndex].Neighbours[i];
                            i32 NeighbourChunkIndex = GetChunkIndexFromCellIndex(Neighbour.Index);
                            if(NeighbourChunkIndex != ChunkIndex && NeighbourChunkIndex != PreviousChunkIndex) {
                                CollisionMeshFromChunk(Grid, NeighbourChunkIndex);
                                PreviousChunkIndex = NeighbourChunkIndex;
                            }
                        }
                    }
                }
                if(Result & EDITSTATE_EDITED_WATER) {
                    TriangulateWaterMesh(Grid, Chunk);
                    i32 PreviousChunkIndex = -1;
                    for(i32 i = 0; i < 6; ++i) {
                        if(Grid->Cells[CellIndex].Neighbours[i]) {
                            hex_cell Neighbour = *Grid->Cells[CellIndex].Neighbours[i];
                            i32 NeighbourChunkIndex = GetChunkIndexFromCellIndex(Neighbour.Index);
                            if(NeighbourChunkIndex != ChunkIndex && NeighbourChunkIndex != PreviousChunkIndex) {
                                TriangulateWaterMesh(Grid, &Grid->Chunks[NeighbourChunkIndex]);
                                PreviousChunkIndex = NeighbourChunkIndex;
                            }
                        }
                    }
                }
                if(Result & EDITSTATE_EDITED_FEATURES) {
                    i32 FeatureDensity = Grid->Cells[CellIndex].FeatureDensity;
                    hex_feature_type FeatureType = Grid->Cells[CellIndex].FeatureType;
                    ClearFeaturesFromCell(&Grid->FeatureSet, &Grid->Cells[CellIndex]);
                    AddFeaturesToCell(&Grid->FeatureSet, &Grid->Cells[CellIndex], FeatureType, FeatureDensity);
                }
            }
            break;
        }
    }

    return CollidedWithThisChunk;
}

internal b32
CheckRayThroughChunk(hex_grid_chunk * Chunk, ray_cast RayCast) {
    for(i32 i = 0; i < 10; ++i) {
        collision_result Result = RayTriangleIntersect(RayCast.Origin, RayCast.Direction, &Chunk->LargeCollisionMesh.Triangles[i]);
        if(Result.DidIntersect) return 1;
    }
    return 0;
}


static void
EditorStateUpdate(app * App) {
    camera * Camera = &App->EditorState.Camera;
    editor_state * EditorState = &App->EditorState;
    hex_grid * Grid = &App->Grid;
    //Note(Zen): Saving and loading keybinds

    if(App->KeyDown[KEY_CTRL] && AppKeyJustDown(KEY_S) && App->KeyDown[KEY_SHIFT]) {
        EditorState->Settings.EditMode = EDIT_MODE_SAVING;
    }
    else if(App->KeyDown[KEY_CTRL] && AppKeyJustDown(KEY_S)) {
        if(*Grid->MapName) {
            SaveGridAsMap(Grid, Grid->MapName, strlen(Grid->MapName));
        }
        else {
            EditorState->Settings.EditMode = EDIT_MODE_SAVING;
        }
    }
    if(App->KeyDown[KEY_CTRL] && AppKeyJustDown(KEY_O)) EditorState->Settings.EditMode = EDIT_MODE_LOADING;
    if(App->KeyDown[KEY_CTRL] && AppKeyJustDown(KEY_N)) EditorState->Settings.EditMode = EDIT_MODE_NEW_MAP;

    static r32 TotalTime = 0.f;
    TotalTime += App->Delta;


    matrix IdentityMatrix = {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    };
    if(EditorState->Settings.EditMode < EDIT_MODE_SAVING) {
        doCamera(Camera, App);

        //Note(Zen): Edit Debug View
        EditorStateDebug.ShowUI ^= AppKeyJustDown(KEY_F1);
        EditorStateDebug.ShowCollisions ^= AppKeyJustDown(KEY_F11);
        EditorStateDebug.ShowLargeCollisions ^= AppKeyJustDown(KEY_F12);
    }
    else if(EditorState->Settings.EditMode == EDIT_MODE_SAVING) {
        CrestUIPushPanel(&App->UI, v2(App->ScreenWidth * 0.5f - 104.f, 200.f), -0.1f);
        CrestUIPushRow(&App->UI, v2(App->ScreenWidth * 0.5f - 104.f, 200.f), v2(100.f, 32.f), 2);

        static i32 FileNameCursor = 0;
        static char FileNameBuffer[32];

        i32 PutCharactersCursor = 0;
        while(App->PutCharacters[PutCharactersCursor] != 0) {
            if(App->PutCharacters[PutCharactersCursor] == '\b' && FileNameCursor > 0) {
                FileNameBuffer[--FileNameCursor] = 0;
                PutCharactersCursor++;
            }
            else if(FileNameCursor < 32) {
                FileNameBuffer[FileNameCursor++] = App->PutCharacters[PutCharactersCursor++];
            }
            else break;
        }

        CrestUITextLabel(&App->UI, GENERIC_ID(0), "Name:");
        CrestUITextLabel(&App->UI, GENERIC_ID(0), FileNameBuffer);


        if(CrestUIButton(&App->UI, GENERIC_ID(0), "Save")) {
            SaveGridAsMap(Grid, FileNameBuffer, FileNameCursor);
            strcpy(Grid->MapName, FileNameBuffer);
            memset(FileNameBuffer, 0, sizeof(FileNameBuffer));
            FileNameCursor = 0;
            EditorState->Settings.EditMode = EDIT_MODE_TERRAIN;
            App->UI.active = CrestUIIDNull();
            App->UI.hot = CrestUIIDNull();
        }
        if(CrestUIButton(&App->UI, GENERIC_ID(0), "Cancel") || App->KeyDown[KEY_ESC]) {
            memset(FileNameBuffer, 0, sizeof(FileNameBuffer));
            FileNameCursor = 0;
            EditorState->Settings.EditMode = EDIT_MODE_TERRAIN;
            App->UI.active = CrestUIIDNull();
            App->UI.hot = CrestUIIDNull();
        }

        CrestUIPopRow(&App->UI);
        CrestUIPopPanel(&App->UI);
    }
    else if(EditorState->Settings.EditMode == EDIT_MODE_LOADING) {
        CrestUIPushPanel(&App->UI, v2(App->ScreenWidth * 0.5f - 104.f, 200.f), -0.1f);
        CrestUIPushRow(&App->UI, v2(App->ScreenWidth * 0.5f - 104.f, 200.f), v2(100.f, 32.f), 2);

        static i32 FileNameCursor = 0;
        static char FileNameBuffer[32];

        i32 PutCharactersCursor = 0;
        while(App->PutCharacters[PutCharactersCursor] != 0) {
            if(App->PutCharacters[PutCharactersCursor] == '\b' && FileNameCursor > 0) {
                FileNameBuffer[--FileNameCursor] = 0;
                PutCharactersCursor++;
            }
            else if(FileNameCursor < 32) {
                FileNameBuffer[FileNameCursor++] = App->PutCharacters[PutCharactersCursor++];
            }
            else break;
        }

        CrestUITextLabel(&App->UI, GENERIC_ID(0), "Name:");
        CrestUITextLabel(&App->UI, GENERIC_ID(0), FileNameBuffer);


        if(CrestUIButton(&App->UI, GENERIC_ID(0), "Load")) {
            LoadGridFromMap(Grid, FileNameBuffer, FileNameCursor);
            strcpy(Grid->MapName, FileNameBuffer);
            ReloadGridVisuals(Grid);
            memset(FileNameBuffer, 0, sizeof(FileNameBuffer));
            FileNameCursor = 0;
            EditorState->Settings.EditMode = EDIT_MODE_TERRAIN;
            App->UI.active = CrestUIIDNull();
            App->UI.hot = CrestUIIDNull();
        }
        if(CrestUIButton(&App->UI, GENERIC_ID(0), "Cancel") || App->KeyDown[KEY_ESC]) {
            memset(FileNameBuffer, 0, sizeof(FileNameBuffer));
            FileNameCursor = 0;
            EditorState->Settings.EditMode = EDIT_MODE_TERRAIN;
            App->UI.active = CrestUIIDNull();
            App->UI.hot = CrestUIIDNull();
        }

        CrestUIPopRow(&App->UI);
        CrestUIPopPanel(&App->UI);
    }
    else if(EditorState->Settings.EditMode == EDIT_MODE_NEW_MAP) {
        ResetCellsOnHexGrid(Grid);
        ReloadGridVisuals(Grid);
        EditorState->Settings.EditMode = EDIT_MODE_TERRAIN;
        memset(Grid->MapName, 0, 32);
    }

    if(EditorStateDebug.ShowUI) {
        EditorState->Settings = doEditorUI(&App->UI, EditorState->Settings, App->ScreenWidth);
        v4 MapNameRect = v4(0, App->ScreenHeight - 32, 128, 32);
        if(*Grid->MapName) {
            CrestUITextLabelP(&App->UI, GENERIC_ID(0), MapNameRect, Grid->MapName);
        }
        else {
            CrestUITextLabelP(&App->UI, GENERIC_ID(0), MapNameRect, "Unsaved Map");
        }
    }
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
    RayClip.x = (2.f * App->Mouse.Position.x) / App->ScreenWidth - 1.f;
    RayClip.y = 1.f - (2.f * App->Mouse.Position.y) / App->ScreenHeight;

    v4 RayEye = CrestMatrixMultipyV4(CrestMatrixInverse(Projection), RayClip);
    RayEye.z = -1.f;
    RayEye.w = 0.f;
    v4 RayWorld = CrestMatrixMultipyV4(CrestMatrixInverse(View), RayEye);
    v3 RayDirection = CrestV3Normalise(v3(RayWorld.x, RayWorld.y, RayWorld.z));

    v3 RayOrigin = GetCameraLocation(Camera);

    ray_cast RayCast = {RayOrigin, RayDirection};

    //Note(Zen): Check for collisions
    char Buffer[32];
    if(App->Mouse.LeftDown && !App->UI.IsMouseOver) {
        for(i32 z = 0; z < HEX_MAX_CHUNKS_HIGH; ++z) {
            for(i32 x = 0; x < HEX_MAX_CHUNKS_WIDE; ++x) {
                i32 Index = z * HEX_MAX_CHUNKS_WIDE + x;
                hex_grid_chunk * Chunk = &Grid->Chunks[Index];

                b32 HitChunk = CheckRayThroughChunk(Chunk, RayCast);
                if(HitChunk) {

                    //CheckCollisionsOnChunk will also edit the cell atm
                    b32 HitCell = EditorCheckCollisionsOnChunk(Index, Grid, EditorState->Settings, RayCast);
                    if(HitCell) goto EditStateEndOfCollisions;
                }

            }
        }
        EditStateEndOfCollisions:;
    }


    CrestShaderSetMatrix(App->Renderer.Shader, "View", &View);
    CrestShaderSetMatrix(App->Renderer.Shader, "Model", &Model);
    CrestShaderSetMatrix(App->Renderer.Shader, "Projection", &Projection);

    //Note(Zen): Draw the Collision Shapes

    if(EditorStateDebug.ShowCollisions) {
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

    if(EditorStateDebug.ShowLargeCollisions) {
        C3DFlush(&App->Renderer);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        for(i32 x = 0; x < HEX_MAX_CHUNKS_WIDE; ++x) {
            for(i32 z = 0; z < HEX_MAX_CHUNKS_HIGH; ++z) {
                for(i32 i = 0; i < 10; ++i) {
                    large_collision_mesh * LargeCollisionMesh = &Grid->Chunks[z * HEX_MAX_CHUNKS_WIDE + x].LargeCollisionMesh;

                    collision_triangle Triangle = LargeCollisionMesh->Triangles[i];
                    C3DDrawTri(&App->Renderer, Triangle.Vertex0, Triangle.Vertex1, Triangle.Vertex2, v3(0.f, 0.9f, 1.f));
                }
            }
        }
        C3DFlush(&App->Renderer);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }



}

internal void
EditorStateFromGameState(editor_state * EditorState, game_state * GameState) {
    EditorState->Camera = GameState->Camera;
    EditorStateDebug.ShowCollisions = GameStateDebug.ShowCollisions;
}


#undef UI_ID_OFFSET
