
internal void
GameStateInit(app * App) {
    game_state * State = &App->GameState;
    State->Camera = CameraInit();

    //Note(Zen): Create the hex mesh
    App->GameState.HexGrid.CollisionMesh.TriangleCount = 0;

    for(i32 z = 0; z < HEX_CHUNK_HEIGHT; ++z) {
        for(i32 x = 0; x < HEX_CHUNK_WIDTH; ++x) {
            i32 Index = z * HEX_CHUNK_WIDTH + x;
            App->GameState.HexGrid.Cells[Index] = CreateCell(x, z);

            v3 Center = App->GameState.HexGrid.Cells[Index].Position;

            i32 Count = App->GameState.HexGrid.CollisionMesh.TriangleCount;
            for(int i = 0; i < 6; ++i) {
                i32 NextIndex = (i + 1) % 6;

                App->GameState.HexGrid.CollisionMesh.Triangles[Count + i] = CreateTriangle(CrestV3Add(Center, HexCorners[i]), CrestV3Add(Center, HexCorners[NextIndex]), Center);
                App->GameState.HexGrid.CollisionMesh.TriangleCount += 1;
            }
        }
    }
}

internal void
GameStateUpdate(app * App) {
    camera * Camera = &App->GameState.Camera;

    static r32 TotalTime = 0.f;
    TotalTime += App->Delta;


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
    //Note(Zen): Clamp camera angle
    if(Camera->Rotation > PI * 0.5f) Camera->Rotation = PI * 0.5f;
    if(Camera->Rotation < PI * 0.25f) Camera->Rotation = PI * 0.25f;


    r32 Ratio = App->ScreenWidth / App->ScreenHeight;
    matrix Projection = CrestMatrixPerspective(PI * 0.5f, Ratio, 0.1f, 100.f);
    matrix View = ViewMatrixFromCamera(Camera);
    matrix Model = IdentityMatrix;
    glUseProgram(App->Renderer.Shader);
    CrestShaderSetMatrix(App->Renderer.Shader, "View", &View);
    CrestShaderSetMatrix(App->Renderer.Shader, "Model", &Model);
    CrestShaderSetMatrix(App->Renderer.Shader, "Projection", &Projection);


    //Note(Zen): Draw the grid.

    hex_grid HexGrid = App->GameState.HexGrid;
    for(i32 CellIndex = 0; CellIndex < HEX_CHUNK_WIDTH * HEX_CHUNK_HEIGHT; ++CellIndex) {
        v3 Center = HexGrid.Cells[CellIndex].Position;
        for(int i = 0; i < 6; ++i) {
            i32 NextIndex = (i + 1) % 6;
            C3DDrawTri(&App->Renderer, Center, CrestV3Add(Center, HexCorners[i]), CrestV3Add(Center, HexCorners[NextIndex]), v3(1.f, 1.f, 1.f));
        }
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

    char Buffer[32];
    sprintf(Buffer, "(%.2f, %.2f, %.2f)", RayDirection.x, RayDirection.y, RayDirection.z);
    CrestUITextLabelP(&App->UI, GENERIC_ID(0), v4(74, 10, 128, 32), Buffer);

    b32 DebugCollisions = 0;
    if(App->KeyDown[KEY_T]) DebugCollisions = 1;
    if(DebugCollisions) {
        C3DFlush(&App->Renderer);
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    }

    for(i32 TriIndex = 0; TriIndex < App->GameState.HexGrid.CollisionMesh.TriangleCount; ++TriIndex) {
        collision_triangle Triangle = App->GameState.HexGrid.CollisionMesh.Triangles[TriIndex];
        collision_result Hit = RayTriangleIntersect(RayOrigin, RayDirection, &Triangle);


        if(Hit.DidIntersect) {
            //Hit.IntersectionPoint
            CrestUITextLabelP(&App->UI, GENERIC_ID(0),  v4(74, 42, 128, 32), "Hit!");
        }

        if(DebugCollisions) {
            C3DDrawTri(&App->Renderer, Triangle.Vertex0, Triangle.Vertex1, Triangle.Vertex2, v3(1.f, 0.f, 0.f));
        }
    }


    if(DebugCollisions) {
        C3DFlush(&App->Renderer);
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }
}
