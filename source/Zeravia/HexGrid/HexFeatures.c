internal hex_feature_set
InitFeatureSet() {
    hex_feature_set Result = {0};
    //load each mesh from a set of paths
    //then put into a vbo
    glGenVertexArrays(HEX_FEATURE_COUNT, Result.VAOs);
    glGenBuffers(HEX_FEATURE_COUNT, Result.VBOs);

    {
        glBindVertexArray(Result.VAOs[HEX_FEATURE_Cube]);
        glBindBuffer(GL_ARRAY_BUFFER, Result.VBOs[HEX_FEATURE_Cube]);

        const float CubeVertices[] = {
            -0.5f, -0.5f, -0.5f,  //0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  //1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  //1.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  //1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  //0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  //0.0f, 0.0f,

            -0.5f, -0.5f,  0.5f,  //0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  //1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  //1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  //1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  //0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  //0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f,  //1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  //1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  //0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  //0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  //0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  //1.0f, 0.0f,

             0.5f,  0.5f,  0.5f,  //1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  //1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  //0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  //0.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  //0.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  //1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,  //0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  //1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  //1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  //1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  //0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  //0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f,  //0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  //1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  //1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  //1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  //0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f  //0.0f, 1.0f
        };

        glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertices), CubeVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(hex_feature), 0);
        glEnableVertexAttribArray(0);
    }
    Result.Shader = CrestShaderInit("../assets/Shaders/hex_feature_shader.vs",
                                    "../assets/Shaders/hex_feature_shader.fs");
    Result.CubeSet[0].Position = v3(0, 0, 0);
    //Result.CubeSet[0].Rotation = 1.f;
    return Result;
}

internal void
DrawFeatureSet(hex_feature_set * Features) {
    glUseProgram(Features->Shader);
    glBindVertexArray(Features->VAOs[0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}
