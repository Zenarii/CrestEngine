internal i32
FeatureIndexFromCell(hex_cell Cell, hex_direction Direction) {
    return Cell.Index * HEX_DIRECTION_COUNT + Direction;
}

internal void
InitFeatureSet(hex_feature_set * Result) {
    //load each mesh from a set of paths
    //then put into a vbo
    glGenVertexArrays(HEX_FEATURE_COUNT, Result->VAOs);
    glGenBuffers(HEX_FEATURE_COUNT, Result->VBOs);
    glGenBuffers(HEX_FEATURE_COUNT, Result->InstancedVBOs);

    for(int i = 1; i < HEX_FEATURE_COUNT; ++i) {
        char * MeshData = CrestLoadFileAsString(HexFeaturePaths[i]);
        mesh Mesh = CrestParseOBJ(MeshData);
        Result->Features[i].MeshVertices = Mesh.VerticesCount;
        free(MeshData);

        glBindVertexArray(Result->VAOs[i]);

        glBindBuffer(GL_ARRAY_BUFFER, Result->VBOs[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Mesh), Mesh.Vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex), 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex), (void *)offsetof(mesh_vertex, Normal));
        glEnableVertexAttribArray(1);


        glBindBuffer(GL_ARRAY_BUFFER, Result->InstancedVBOs[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(matrix) * MAX_FEATURE_SET_SIZE, &Result->Features[i].Model[0], GL_DYNAMIC_DRAW);

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
    Result->Shader = CrestShaderInit("../assets/Shaders/hex_feature_shader.vs",
                                    "../assets/Shaders/hex_feature_shader.fs");

    CrestShaderSetV3(Result->Shader, "Material.Ambient", v3(1.f, 1.f, 1.f));
    CrestShaderSetV3(Result->Shader, "Material.Diffuse", v3(1.f, 1.f, 1.f));
    CrestShaderSetV3(Result->Shader, "Material.Specular", v3(1.f, 1.f, 1.f));
    CrestShaderSetFloat(Result->Shader, "Material.Shininess", 8.f);

}

internal v3
NudgeFeature(v3 Position) {
    v3 ScaledPosition = CrestV3Scale(Position, FEATURES_NOISE_SCALE);
    v3 Sample = NoiseRandom3DSample(ScaledPosition);
    //Sample in range [0..1] so convert to [-1..1]
    Sample.y = 0;
    Sample.x = 2 * Sample.x - 1;
    Sample.z = 2 * Sample.z - 1;
    Sample = CrestV3Scale(Sample, FEATURES_NUDGE_STRENGTH);
    v3 Result = CrestV3Add(Position, Sample);
    return Result;
}

//TODO Allow for directional features
//to allow for decorations such as ports etc

internal void
AddFeaturesToCell(hex_feature_set * Set, hex_cell * Cell, hex_feature_type Type, i32 Density) {
    Cell->FeatureDensity = Density;
    Cell->FeatureType = Type;
    i32 DirectionsList[] = {0, 1, 2, 3, 4, 5};
    CrestShuffleArray(DirectionsList, 6);
    //DirectionsList[0] = 0;
    for(i32 i = 0; i < Density; ++i) {
        hex_direction Direction = DirectionsList[i];
        Cell->Features[Direction] = Type;
        v3 ModelPosition = CrestV3Add(Cell->Position, CrestV3Scale(HexCorners[Direction], HEX_SOLID_FACTOR * 0.5f));
        ModelPosition = NudgeFeature(ModelPosition);
        r32 ModelRotation = PI + (Direction * PI/3.f);//(RandomNoise3D(ModelPosition) * 2 - 1) * PI;
        r32 ModelScale = 0.9f + 0.2f * RandomNoise3D(CrestV3Scale(ModelPosition, 2.f * FEATURES_NOISE_SCALE));
        matrix Model = CrestM4MultM4(CrestMatrixRotation(ModelRotation, CREST_AXIS_Y), CrestMatrixScale(ModelScale));
        Model = CrestM4MultM4(CrestMatrixTranslation(ModelPosition), Model);
        Set->Features[Type].Model[Cell->Index * HEX_DIRECTION_COUNT + Direction] = CrestMatrixTranspose(Model);
    }

    glBindVertexArray(Set->VAOs[Type]);
    glBindBuffer(GL_ARRAY_BUFFER, Set->InstancedVBOs[Type]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(matrix) * MAX_FEATURE_SET_SIZE, &Set->Features[Type].Model[0]);

}

internal void
ClearFeaturesFromCell(hex_feature_set * Set, hex_cell * Cell) {
    for(hex_direction Direction = 0; Direction < HEX_DIRECTION_COUNT; ++Direction) {
        hex_feature_type Type = Cell->Features[Direction];
        Set->Features[Type].Model[Cell->Index * HEX_DIRECTION_COUNT + Direction] = CrestMatrixZero();
        Cell->Features[Direction] = 0;

    }

    glBindVertexArray(Set->VAOs[Cell->FeatureType]);
    glBindBuffer(GL_ARRAY_BUFFER, Set->InstancedVBOs[Cell->FeatureType]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(matrix) * MAX_FEATURE_SET_SIZE, &Set->Features[Cell->FeatureType].Model[0]);


    Cell->FeatureDensity = 0;
    Cell->FeatureType = 0;
}



internal void
DrawFeatureSet(hex_feature_set * FeatureSet) {
    glUseProgram(FeatureSet->Shader);
    for(i32 i = 1; i < HEX_FEATURE_COUNT; ++i) {
        glBindVertexArray(FeatureSet->VAOs[i]);
        glDrawArraysInstanced(GL_TRIANGLES, 0, FeatureSet->Features[i].MeshVertices, MAX_FEATURE_SET_SIZE);
    }
}
