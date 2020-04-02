#define UI_ID_OFFSET 1000

internal void
EditorStateInit(app * App) {
    editor_state * State = &App->EditorState;
    State->Camera = CameraInit();

    //Note(Zen): Create the hex mesh
    App->EditorState.HexGrid.CollisionMesh.TriangleCount = 0;

    for(i32 z = 0; z < HEX_CHUNK_HEIGHT; ++z) {
        for(i32 x = 0; x < HEX_CHUNK_WIDTH; ++x) {
            i32 Index = z * HEX_CHUNK_WIDTH + x;
            App->EditorState.HexGrid.Cells[Index] = CreateCell(x, z);


            //Note(Zen): Create the collision mesh
            v3 Center = App->EditorState.HexGrid.Cells[Index].Position;

            i32 Count = App->EditorState.HexGrid.CollisionMesh.TriangleCount;
            for(int i = 0; i < 6; ++i) {
                i32 NextIndex = (i + 1) % 6;

                App->EditorState.HexGrid.CollisionMesh.Triangles[Count + i] = CreateTriangle(CrestV3Add(Center, HexCorners[i]), CrestV3Add(Center, HexCorners[NextIndex]), Center);
                App->EditorState.HexGrid.CollisionMesh.TriangleCount += 1;
            }
        }
    }

    TriangulateMesh(&App->EditorState.HexGrid);

    //Set default editor settings
    State->Settings.Colour = *EditorColourV;
}

internal hex_edit_settings
doEditorUI(CrestUI * ui, hex_edit_settings Settings) {

    CrestUIPushPanel(ui, v2(10.f, 10.f), -0.1f);
    CrestUIPushRow(ui, v2(10.f, 10.f), v2(128, 32), 1);
    /*
        Colours
    */

    for(i32 ColourIndex = 0; ColourIndex < EDITOR_COLOUR_COUNT; ++ColourIndex) {
        Settings.Colour = CrestUIButton(ui, GENERIC_ID(ColourIndex), EditorColourString[ColourIndex])
                              ? EditorColourV[ColourIndex] : Settings.Colour;
    }

    /*
        Elevation
    */

    CrestUIPopRow(ui);
    CrestUIPopPanel(ui);

    return Settings;
}



global b32 DebugCollisions = 1;

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
    if(App->KeyDown[KEY_W]) Camera->Position.z -= CAMERA_SPEED * App->Delta;
    if(App->KeyDown[KEY_S]) Camera->Position.z += CAMERA_SPEED * App->Delta;
    if(App->KeyDown[KEY_A]) Camera->Position.x -= CAMERA_SPEED * App->Delta;
    if(App->KeyDown[KEY_D]) Camera->Position.x += CAMERA_SPEED * App->Delta;
    if(App->KeyDown[KEY_Q]) Camera->Rotation   += 3 * App->Delta;
    if(App->KeyDown[KEY_E]) Camera->Rotation   -= 3 * App->Delta;
    //Note(Zen): Clamp camera angle
    if(Camera->Rotation > PI * 0.5f) Camera->Rotation = PI * 0.5f;
    if(Camera->Rotation < PI * 0.25f) Camera->Rotation = PI * 0.25f;


    EditorState->Settings = doEditorUI(&App->UI, EditorState->Settings);


    //Note(Zen): Draw the grid.
    r32 Ratio = App->ScreenWidth / App->ScreenHeight;
    matrix Projection = CrestMatrixPerspective(PI * 0.5f, Ratio, 0.1f, 100.f);
    matrix View = ViewMatrixFromCamera(Camera);
    matrix Model = IdentityMatrix;
    glUseProgram(App->Renderer.Shader);
    CrestShaderSetMatrix(App->Renderer.Shader, "View", &View);
    CrestShaderSetMatrix(App->Renderer.Shader, "Model", &Model);
    CrestShaderSetMatrix(App->Renderer.Shader, "Projection", &Projection);

    hex_mesh HexMesh = App->EditorState.HexGrid.HexMesh;
    DrawHexMesh(&App->Renderer, &HexMesh);

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

    char Buffer[32];
    if(App->LeftMouseDown && !App->UI.IsMouseOver) {
        for(i32 TriIndex = 0; TriIndex < App->EditorState.HexGrid.CollisionMesh.TriangleCount; ++TriIndex) {
            collision_triangle Triangle = App->EditorState.HexGrid.CollisionMesh.Triangles[TriIndex];
            collision_result Hit = RayTriangleIntersect(RayOrigin, RayDirection, &Triangle);


            if(Hit.DidIntersect) {
                hex_coordinates SelectedHex = CartesianToHexCoords(Hit.IntersectionPoint.x, Hit.IntersectionPoint.z);
                sprintf(Buffer, "(%d, %d, %d)", SelectedHex.x, SelectedHex.y, SelectedHex.z);
                CrestUITextLabelP(&App->UI, GENERIC_ID(0),  v4(74, 42, 128, 32), Buffer);

                //change the colour
                i32 Index = GetCellIndex(SelectedHex);
                if(Index > -1) {
                    App->EditorState.HexGrid.Cells[Index].Colour = EditorState->Settings.Colour;
                    TriangulateMesh(&App->EditorState.HexGrid);
                }
                else {
                    OutputDebugStringA("Cell Index out of bounds");
                }
            }
        }
    }

    //Note(Zen): Draw the Collision Shapes
    DebugCollisions ^= App->KeyDown[KEY_T];
    if(DebugCollisions) {
        C3DFlush(&App->Renderer);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        for(i32 TriIndex = 0; TriIndex < App->EditorState.HexGrid.CollisionMesh.TriangleCount; ++TriIndex) {
            collision_triangle Triangle = App->EditorState.HexGrid.CollisionMesh.Triangles[TriIndex];
            C3DDrawTri(&App->Renderer, Triangle.Vertex0, Triangle.Vertex1, Triangle.Vertex2, v3(1.f, 0.f, 0.f));
        }
        C3DFlush(&App->Renderer);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}
#undef UI_ID_OFFSET
