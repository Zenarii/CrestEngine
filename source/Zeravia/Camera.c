internal camera
CameraInit() {
    camera Result = {0};
    Result.Rotation = PI * 0.45f;
    Result.Translation = v3(0.f, 16.f, 0.f);
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
    matrix CameraSpace = CrestM4MultM4(RotationMatrix, TranslationMatrix);
    matrix WorldSpace = CrestM4MultM4(SwivelMatrix, PositionMatrix);
    return CrestM4MultM4(CameraSpace, WorldSpace);
}

internal void
doCamera(camera * Camera, app * App) {
    if(App->KeyDown[KEY_W]) {
        Camera->Position.z -= cosf(Camera->Swivel) * CAMERA_SPEED * App->Delta;
        Camera->Position.x += sinf(Camera->Swivel) * CAMERA_SPEED * App->Delta;
    }
    if(App->KeyDown[KEY_S]) {
        Camera->Position.z += cosf(Camera->Swivel) * CAMERA_SPEED * App->Delta;
        Camera->Position.x -= sinf(Camera->Swivel) * CAMERA_SPEED * App->Delta;
    }
    if(App->KeyDown[KEY_A]) {
        Camera->Position.z -= sinf(Camera->Swivel) * CAMERA_SPEED * App->Delta;
        Camera->Position.x -= cosf(Camera->Swivel) * CAMERA_SPEED * App->Delta;
    }
    if(App->KeyDown[KEY_D]) {
        Camera->Position.z += sinf(Camera->Swivel) * CAMERA_SPEED * App->Delta;
        Camera->Position.x += cosf(Camera->Swivel) * CAMERA_SPEED * App->Delta;
    }


    if(App->KeyDown[KEY_Q]) Camera->Swivel += App->Delta *  0.5f * PI;
    if(App->KeyDown[KEY_E]) Camera->Swivel -= App->Delta *  0.5f *PI;

    if(Camera->Rotation > 0.45f * PI) Camera->Rotation = 0.45f * PI;
    if(Camera->Rotation <= 0.2f * PI) Camera->Rotation = 0.2f * PI;
}

internal v3
GetCameraLocation(camera * Camera) {
    return CrestV3Add(Camera->Position, Camera->Translation);
}
