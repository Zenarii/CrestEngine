internal camera
CameraInit() {
    camera Result = {PI * 0.5f, v3(0.f, 8.f, 0.f)};
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
