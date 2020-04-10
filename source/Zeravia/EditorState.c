#define UI_ID_OFFSET 1000

internal void
EditorStateInit(app * App) {
    editor_state * State = &App->EditorState;
    State->Camera = CameraInit();


    hex_grid * Grid = &App->EditorState.HexGrid;
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
    Grid->Height = HEX_MAX_CHUNKS_HIGH * HEX_CHUNK_HEIGHT;
    AddCellsToHexGrid(Grid);
    hex_cell * Cells = App->EditorState.HexGrid.Cells;
    for(i32 x = 0; x < HEX_MAX_CHUNKS_WIDE; ++x) {
        for(i32 z = 0; z < HEX_MAX_CHUNKS_HIGH; ++z) {
            hex_grid_chunk * Chunk = &App->EditorState.HexGrid.Chunks[z * HEX_MAX_CHUNKS_WIDE + x];
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

    //Set default editor settings
    State->Settings.EditColour = 0;
    State->Settings.Colour = EditorColourV[EDITOR_COLOUR_COUNT-1];
    State->Settings.EditElevation = 0;
    State->Settings.Elevation = 0;
    State->Settings.BrushSize = 0;
    State->Settings.EditWater = 0;
    State->Settings.WaterLevel = 0;
}

internal hex_edit_settings
doEditorUITerrain(CrestUI * ui, hex_edit_settings Settings) {
    Settings.EditColour = CrestUIToggleButton(ui, GENERIC_ID(0), Settings.EditColour, "Colour");
    if(Settings.EditColour) {
        for(i32 ColourIndex = 0; ColourIndex < EDITOR_COLOUR_COUNT; ++ColourIndex) {
            Settings.Colour = CrestUIButton(ui, GENERIC_ID(ColourIndex), EditorColourString[ColourIndex])
                                  ? EditorColourV[ColourIndex] : Settings.Colour;
        }
    }
    /*
        Elevation
    */
    Settings.EditElevation = CrestUIToggleButton(ui, GENERIC_ID(0), Settings.EditElevation, "Elevation");
    if(Settings.EditElevation) {
        Settings.Elevation = CrestUISliderInt(ui, GENERIC_ID(0), Settings.Elevation, HEX_MAX_ELEVATION, "Elevation");
    }
    Settings.BrushSize = 0;//CrestUISliderInt(ui, GENERIC_ID(0), Settings.BrushSize, 4, "BrushSize");

    return Settings;
}

internal hex_edit_settings
doEditorUITerrainFeatures(CrestUI * ui, hex_edit_settings Settings) {
    Settings.EditWater = CrestUIToggleButton(ui, GENERIC_ID(0), Settings.EditWater, "Water:");
    if(Settings.EditWater) Settings.WaterLevel = CrestUISliderInt(ui, GENERIC_ID(0), Settings.WaterLevel, HEX_MAX_ELEVATION, "Water Level");

    return Settings;
}

internal hex_edit_settings
doEditorUI(CrestUI * ui, hex_edit_settings Settings, r32 ScreenWidth) {
    CrestUIPushPanel(ui, v2(1.f, 1.f), -0.1f);
    CrestUIPushRow(ui, v2(1.f, 1.f), v2(150, 32.f), EDIT_MODE_COUNT);
    {
        Settings.EditMode = CrestUIButton(ui, GENERIC_ID(0), "Terrain") ? EDIT_MODE_TERRAIN : Settings.EditMode;
        Settings.EditMode = CrestUIButton(ui, GENERIC_ID(0), "Features") ? EDIT_MODE_TERRAIN_FEATURES : Settings.EditMode;
    }
    CrestUIPopRow(ui);
    CrestUIPopPanel(ui);

    CrestUIPushPanel(ui, v2(10.f, 47.f), -0.1f);
    CrestUIPushRow(ui, v2(10.f, 47.f), v2(150, 32), 1);
    /*
        Colours
    */
    if(Settings.EditMode == EDIT_MODE_TERRAIN) Settings = doEditorUITerrain(ui, Settings);
    else if (Settings.EditMode == EDIT_MODE_TERRAIN_FEATURES) Settings = doEditorUITerrainFeatures(ui, Settings);

    CrestUIPopRow(ui);
    CrestUIPopPanel(ui);

    return Settings;
}

internal b32
EditCellTerrain(hex_cell * Cell, hex_edit_settings Settings) {
    b32 Result = 0;
    if(Settings.EditColour && !CrestV3Equals(Cell->Colour, Settings.Colour)) {
        Cell->Colour = Settings.Colour;
        Result |= EDITSTATE_EDITED_MESH;
    }
    if(Settings.EditElevation && (Cell->Elevation != Settings.Elevation)) {
        Cell->Elevation = Settings.Elevation;
        Cell->Position.y = Cell->Elevation * HEX_ELEVATION_STEP;
        v3 Sample = Noise3DSample(Cell->Position);
        Cell->Position.y += Sample.y * HEX_ELEVATION_NUDGE_STRENGTH;
        Result |= (EDITSTATE_EDITED_COLLISIONS | EDITSTATE_EDITED_MESH | EDITSTATE_EDITED_WATER);
    }
    return Result;
}

internal b32
EditCellTerrainFeatures(hex_cell * Cell, hex_edit_settings Settings) {
    b32 Result = 0;
    if(Settings.EditWater && Settings.WaterLevel != Cell->WaterLevel) {
        Cell->WaterLevel = Settings.WaterLevel;
        Result |= EDITSTATE_EDITED_WATER;
    }
    return Result;
}



internal b32
EditCell(hex_cell * Cell, hex_edit_settings Settings) {
    b32 Result = 0;
    if(Settings.EditMode == EDIT_MODE_TERRAIN) Result |= EditCellTerrain(Cell, Settings);
    else if(Settings.EditMode == EDIT_MODE_TERRAIN_FEATURES) Result |= EditCellTerrainFeatures(Cell, Settings);
    return Result;
}

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

internal b32
CheckCollisionsOnChunk(i32 ChunkIndex, hex_grid * Grid, hex_edit_settings Settings, ray_cast RayCast) {
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
                b32 Result = EditCell(&Grid->Cells[CellIndex], Settings);
                if(Result & EDITSTATE_EDITED_MESH) {
                    TriangulateMesh(Grid, Chunk);
                    //Note(Zen): Edit any neighbouring chunks that require it
                    for(i32 i = 0; i < 6; ++i) {
                        if(Grid->Cells[CellIndex].Neighbours[i]) {
                            hex_cell Neighbour = *Grid->Cells[CellIndex].Neighbours[i];
                            i32 NeighbourChunkIndex = GetChunkIndexFromCellIndex(Neighbour.Index);
                            if(NeighbourChunkIndex != ChunkIndex) {
                                TriangulateMesh(Grid, &Grid->Chunks[NeighbourChunkIndex]);
                            }
                        }
                    }
                }
                if(Result & EDITSTATE_EDITED_COLLISIONS) {
                    CollisionMeshFromChunk(Grid, ChunkIndex);
                    for(i32 i = 0; i < 6; ++i) {
                        if(Grid->Cells[CellIndex].Neighbours[i]) {
                            hex_cell Neighbour = *Grid->Cells[CellIndex].Neighbours[i];
                            i32 NeighbourChunkIndex = GetChunkIndexFromCellIndex(Neighbour.Index);
                            if(NeighbourChunkIndex != ChunkIndex) {
                                CollisionMeshFromChunk(Grid, NeighbourChunkIndex);
                            }
                        }
                    }
                }
                if(Result & EDITSTATE_EDITED_WATER) {
                    TriangulateWaterMesh(Grid, Chunk);
                    for(i32 i = 0; i < 6; ++i) {
                        if(Grid->Cells[CellIndex].Neighbours[i]) {
                            hex_cell Neighbour = *Grid->Cells[CellIndex].Neighbours[i];
                            i32 NeighbourChunkIndex = GetChunkIndexFromCellIndex(Neighbour.Index);
                            if(NeighbourChunkIndex != ChunkIndex) {
                                TriangulateWaterMesh(Grid, &Grid->Chunks[NeighbourChunkIndex]);
                            }
                        }
                    }
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

global b32 DebugCollisions = 0;
global b32 DebugLargeCollions = 0;

static void
EditorStateUpdate(app * App) {
    camera * Camera = &App->EditorState.Camera;
    editor_state * EditorState = &App->EditorState;

    static r32 TotalTime = 0.f;
    TotalTime += App->Delta;


    matrix IdentityMatrix = {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    };

    //Note(Zen): Camera Controls
    if(App->KeyDown[KEY_W]) {
        Camera->Position.z -= CAMERA_SPEED * App->Delta;
        OutputDebugStringA("W");
    }
    if(App->KeyDown[KEY_S]) Camera->Position.z += CAMERA_SPEED * App->Delta;
    if(App->KeyDown[KEY_A]) Camera->Position.x -= CAMERA_SPEED * App->Delta;
    if(App->KeyDown[KEY_D]) Camera->Position.x += CAMERA_SPEED * App->Delta;
    if(App->KeyDown[KEY_Q]) Camera->Rotation   += 3 * App->Delta;
    if(App->KeyDown[KEY_E]) Camera->Rotation   -= 3 * App->Delta;
    //Note(Zen): Clamp camera angle
    if(Camera->Rotation > PI * 0.5f) Camera->Rotation = PI * 0.5f;
    if(Camera->Rotation < PI * 0.1f) Camera->Rotation = PI * 0.1f;


    EditorState->Settings = doEditorUI(&App->UI, EditorState->Settings, App->ScreenWidth);


    //Note(Zen): Draw the grid.
    r32 Ratio = App->ScreenWidth / App->ScreenHeight;
    matrix Projection = CrestMatrixPerspective(PI * 0.5f, Ratio, 0.1f, 100.f);
    matrix View = ViewMatrixFromCamera(Camera);
    matrix Model = IdentityMatrix;

    CrestShaderSetMatrix(EditorState->HexGrid.MeshShader, "View", &View);
    CrestShaderSetMatrix(EditorState->HexGrid.MeshShader, "Model", &Model);
    CrestShaderSetMatrix(EditorState->HexGrid.MeshShader, "Projection", &Projection);

    CrestShaderSetV3(EditorState->HexGrid.MeshShader, "ViewPosition", Camera->Position);
    CrestShaderSetV3(EditorState->HexGrid.MeshShader, "LightColour", v3(1.f, 1.f, 1.f));
    CrestShaderSetV3(EditorState->HexGrid.MeshShader, "LightPosition", v3(3.f, 8.f, 3.f));

    CrestShaderSetMatrix(EditorState->HexGrid.WaterShader, "View", &View);
    CrestShaderSetMatrix(EditorState->HexGrid.WaterShader, "Model", &Model);
    CrestShaderSetMatrix(EditorState->HexGrid.WaterShader, "Projection", &Projection);

    CrestShaderSetV3(EditorState->HexGrid.WaterShader, "ViewPosition", Camera->Position);
    CrestShaderSetV3(EditorState->HexGrid.WaterShader, "LightColour", v3(1.f, 1.f, 1.f));
    CrestShaderSetV3(EditorState->HexGrid.WaterShader, "LightPosition", v3(3.f, 8.f, 3.f));
    CrestShaderSetFloat(EditorState->HexGrid.WaterShader, "Time", App->TotalTime);

    for(i32 i = 0; i < HEX_MAX_CHUNKS; ++i) {
        hex_mesh * HexMesh = &App->EditorState.HexGrid.Chunks[i].HexMesh;
        DrawHexMesh(&App->EditorState.HexGrid, HexMesh);
    }
    for(i32 i = 0; i < HEX_MAX_CHUNKS; ++i) {
        hex_mesh * WaterMesh = &App->EditorState.HexGrid.Chunks[i].WaterMesh;
        DrawWaterMesh(&App->EditorState.HexGrid, WaterMesh);
    }

    //Note(Zen): Check collisions
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

    //Note(Zen): Check for collisions
    char Buffer[32];
    if(App->LeftMouseDown && !App->UI.IsMouseOver) {
        for(i32 z = 0; z < HEX_MAX_CHUNKS_HIGH; ++z) {
            for(i32 x = 0; x < HEX_MAX_CHUNKS_WIDE; ++x) {
                i32 Index = z * HEX_MAX_CHUNKS_WIDE + x;
                hex_grid_chunk * Chunk = &App->EditorState.HexGrid.Chunks[Index];

                b32 HitChunk = CheckRayThroughChunk(Chunk, RayCast);
                if(HitChunk) {
                    //CheckCollisionsOnChunk will also edit the cell atm
                    b32 HitCell = CheckCollisionsOnChunk(Index, &App->EditorState.HexGrid, EditorState->Settings, RayCast);
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
    DebugCollisions ^= AppKeyJustDown(KEY_T);
    if(DebugCollisions) {
        C3DFlush(&App->Renderer);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        for(i32 x = 0; x < HEX_MAX_CHUNKS_WIDE; ++x) {
            for(i32 z = 0; z < HEX_MAX_CHUNKS_HIGH; ++z) {
                collision_mesh * CollisionMesh = &App->EditorState.HexGrid.Chunks[z * HEX_MAX_CHUNKS_WIDE + x].CollisionMesh;
                for(i32 TriIndex = 0; TriIndex < CollisionMesh->TriangleCount; ++TriIndex) {
                    collision_triangle Triangle = CollisionMesh->Triangles[TriIndex];
                    C3DDrawTri(&App->Renderer, Triangle.Vertex0, Triangle.Vertex1, Triangle.Vertex2, v3(1.f, 0.f, 0.f));
                }
            }
        }
        C3DFlush(&App->Renderer);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    DebugLargeCollions ^= AppKeyJustDown(KEY_P);
    if(DebugLargeCollions) {
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

}
#undef UI_ID_OFFSET
