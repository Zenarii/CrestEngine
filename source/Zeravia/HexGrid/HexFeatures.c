internal hex_feature_set
InitFeatureSet() {
    hex_feature_set Result = {0};
    //load each mesh from a set of paths
    //then put into a vbo
    glGenVertexArrays(HEX_FEATURE_COUNT, Result.VAOs);
    glGenBuffers(HEX_FEATURE_COUNT, Result.VBOs);
    glGenBuffers(HEX_FEATURE_COUNT, Result.InstancedVBOs);

    for(int i = 1; i < HEX_FEATURE_COUNT; ++i) {
        char * MeshData = CrestLoadFileAsString("../assets/FeatureModels/tree.obj");
        mesh Mesh = CrestParseOBJ(MeshData);
        Result.Features[i].MeshVertices = Mesh.VerticesCount;
        free(MeshData);
        Result.Features[i].Count = 0;

        glBindVertexArray(Result.VAOs[i]);

        glBindBuffer(GL_ARRAY_BUFFER, Result.VBOs[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Mesh), Mesh.Vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex), 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex), (void *)offsetof(mesh_vertex, Normal));
        glEnableVertexAttribArray(1);


        glBindBuffer(GL_ARRAY_BUFFER, Result.InstancedVBOs[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(matrix) * MAX_FEATURE_SET_SIZE, &Result.Features[i].Model[0], GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), 0);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void *)sizeof(v4));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void *)(2*sizeof(v4)));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void *)(3*sizeof(v4)));

        glVertexAttribDivisor(2, 1);
        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);

    }
    Result.Shader = CrestShaderInit("../assets/Shaders/hex_feature_shader.vs",
                                    "../assets/Shaders/hex_feature_shader.fs");

    return Result;
}

internal void
AddFeaturesToCell(hex_feature_set * Set, hex_cell * Cell, hex_feature_type Type) {
    //Add Mesh
    hex_feature * Feature = &Set->Features[Type];
    if(Feature->Count >= MAX_FEATURE_SET_SIZE) return; //TODO(Zen): Logging
    matrix Model = CrestMatrixTranslation(Cell->Position);
    Feature->Model[Feature->Count] = CrestMatrixTranspose(Model);
    glBindVertexArray(Set->VAOs[Type]);
    glBindBuffer(GL_ARRAY_BUFFER, Set->InstancedVBOs[Type]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (Feature->Count + 1) * sizeof(matrix), &Feature->Model[0]);

    //Adjust Cell
    Cell->FeatureType = Type;
    Cell->FeatureIndex = Feature->Count;

    Feature->Count++;
}

internal void
ClearFeaturesFromCell(hex_feature_set * Set, hex_cell * Cell, hex_feature_type Type) {
    hex_feature * Feature = &Set->Features[Type];
    Assert(Feature->Count != 0);
    Feature->Model[Cell->FeatureIndex] = Feature->Model[--Feature->Count];

    Cell->FeatureType = 0;
    Cell->FeatureIndex = 0;

}



internal void
DrawFeatureSet(hex_feature_set * FeatureSet) {
    glUseProgram(FeatureSet->Shader);
    for(i32 i = 1; i < HEX_FEATURE_COUNT; ++i) {
        glBindVertexArray(FeatureSet->VAOs[i]);
        glDrawArraysInstanced(GL_TRIANGLES, 0, FeatureSet->Features[i].MeshVertices, FeatureSet->Features[i].Count);
    }
}
