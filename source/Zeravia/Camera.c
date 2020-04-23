internal camera
CameraInit() {
    camera Result = {0};
    Result.Rotation = PI * 0.45f;
    Result.Translation = v3(0.f, 16.f, 0.f);
    Result.Zoom = 1.f;
    return Result;
}

internal matrix
ViewMatrixFromCamera(camera * Camera) {
    matrix RotationMatrix = CrestMatrixRotation(Camera->Rotation, CREST_AXIS_X);
    matrix TranslationMatrix = CrestMatrixTranslation(v3(-Camera->Translation.x,
                                                         -Camera->Translation.y,
                                                         -Camera->Translation.z));
    matrix SwivelMatrix = CrestMatrixRotation(Camera->Swivel, CREST_AXIS_Y);
    matrix PositionMatrix = CrestMatrixTranslation(v3(-Camera->Position.x,
                                                      -Camera->Position.y,
                                                      -Camera->Position.z));
    matrix CameraSpace = CrestM4MultM4(TranslationMatrix, RotationMatrix);
    matrix WorldSpace = CrestM4MultM4(SwivelMatrix, PositionMatrix);
    return CrestM4MultM4(CameraSpace, WorldSpace);
}

internal void
doCamera(camera * Camera, app * App) {
    //Note(Zen): Zoom
    {
        Camera->Zoom -= App->Mouse.Scroll * CAMERA_ZOOM_SPEED * App->Delta;
        if(Camera->Zoom > 1.f) Camera->Zoom = 1.f;
        if(Camera->Zoom < 0.f) Camera->Zoom = 0.f;
        Camera->Translation = CrestV3Lerp(v3(0.f, 0.f, CAMERA_MIN_DISTANCE), v3(0.f, 0.f, CAMERA_MAX_DISTANCE), Camera->Zoom);
        Camera->Rotation = CrestLerp(CAMERA_MIN_ANGLE, CAMERA_MAX_ANGLE, Camera->Zoom);
        Camera->Speed = CrestLerp(CAMERA_IN_SPEED, CAMERA_OUT_SPEED, Camera->Zoom);
    }

    //Note(Zen): Movement Controls
    {
        if(App->KeyDown[KEY_W]) {
            Camera->Position.z -= cosf(Camera->Swivel) * Camera->Speed * App->Delta;
            Camera->Position.x += sinf(Camera->Swivel) * Camera->Speed * App->Delta;
        }
        if(App->KeyDown[KEY_S]) {
            Camera->Position.z += cosf(Camera->Swivel) * Camera->Speed * App->Delta;
            Camera->Position.x -= sinf(Camera->Swivel) * Camera->Speed * App->Delta;
        }
        if(App->KeyDown[KEY_A]) {
            Camera->Position.z -= sinf(Camera->Swivel) * Camera->Speed * App->Delta;
            Camera->Position.x -= cosf(Camera->Swivel) * Camera->Speed * App->Delta;
        }
        if(App->KeyDown[KEY_D]) {
            Camera->Position.z += sinf(Camera->Swivel) * Camera->Speed * App->Delta;
            Camera->Position.x += cosf(Camera->Swivel) * Camera->Speed * App->Delta;
        }


        if(App->KeyDown[KEY_Q]) Camera->Swivel -= App->Delta * 0.25f * PI;
        if(App->KeyDown[KEY_E]) Camera->Swivel += App->Delta * 0.25f * PI;
    }
}

internal v3
GetCameraLocation(camera * Camera) {
    v3 Base = Camera->Position;
    r32 Height = sin(Camera->Rotation) * Camera->Translation.z;
    r32 FromBase = cos(Camera->Rotation) * Camera->Translation.z;

    v3 Offset = v3(-sin(Camera->Swivel) * FromBase, Height, cos(Camera->Swivel) * FromBase);

    return CrestV3Add(Base, Offset);
}
