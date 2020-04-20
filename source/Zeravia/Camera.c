internal camera
CameraInit() {
    camera Result = {PI * 0.3f, v3(0.f, 16.f, 0.f)};
    return Result;
}

internal matrix
ViewMatrixFromCamera(camera * Camera) {
    matrix RotationMatrix = CrestMatrixRotation(Camera->Rotation, CREST_AXIS_X);
    matrix TranslationMatrix = CrestMatrixTranslation(v3(-Camera->Position.x,
                                                         -Camera->Position.y,
                                                         -Camera->Position.z));

    return CrestM4MultM4(RotationMatrix, TranslationMatrix);
}

internal void
doCamera(camera * Camera, app * App) {
    if(App->KeyDown[KEY_W]) Camera->Position.z -= CAMERA_SPEED * App->Delta;
    if(App->KeyDown[KEY_S]) Camera->Position.z += CAMERA_SPEED * App->Delta;
    if(App->KeyDown[KEY_A]) Camera->Position.x -= CAMERA_SPEED * App->Delta;
    if(App->KeyDown[KEY_D]) Camera->Position.x += CAMERA_SPEED * App->Delta;
}
