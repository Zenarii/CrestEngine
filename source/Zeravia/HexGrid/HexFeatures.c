internal hex_feature_set
InitFeatureSet() {
    hex_feature_set Result = {0};
    //load each mesh from a set of paths
    //then put into a vbo
    glGenVertexArrays(HEX_FEATURE_COUNT, Result.VAOs);
    glGenBuffers(HEX_FEATURE_COUNT, Result.VBOs);
    glGenBuffers(HEX_FEATURE_COUNT, Result.InstancedVBOs);

    {
        char * CubeData = CrestLoadFileAsString("../assets/FeatureModels/Cube.obj");
        mesh CubeMesh = CrestParseOBJ(CubeData);
        free(CubeData);

        for(i32 i = 0; i < 15; ++i) {
            Result.CubeSet[i] = CrestM4MultM4(CrestMatrixTranslation(v3(i, 0, i)), CrestMatrixRotation(i, CREST_AXIS_Y));
            Result.CubeSet[i] = CrestMatrixTranspose(Result.CubeSet[i]);
        }

        glBindVertexArray(Result.VAOs[HEX_FEATURE_Cube]);

        glBindBuffer(GL_ARRAY_BUFFER, Result.VBOs[HEX_FEATURE_Cube]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(CubeMesh), CubeMesh.Vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex), 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, Result.InstancedVBOs[HEX_FEATURE_Cube]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(matrix) * 15, &Result.CubeSet[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), 0);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void *)sizeof(v4));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void *)(2*sizeof(v4)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void *)(3*sizeof(v4)));

        glVertexAttribDivisor(1, 1);
        glVertexAttribDivisor(2, 1);
        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);

    }
    Result.Shader = CrestShaderInit("../assets/Shaders/hex_feature_shader.vs",
                                    "../assets/Shaders/hex_feature_shader.fs");

    return Result;
}

internal void
DrawFeatureSet(hex_feature_set * Features) {
    glUseProgram(Features->Shader);
    glBindVertexArray(Features->VAOs[0]);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, 15);
}
