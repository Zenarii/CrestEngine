internal i32
FeatureIndexFromCell(hex_cell Cell, hex_direction Direction) {
    return Cell.Index * HEX_DIRECTION_COUNT + Direction;
}

internal void
InitFeatureSet(hex_feature_set * Result) {
    //Note(Zen): load each mesh from a set of path then put into a vbo
    glGenVertexArrays(HEX_FEATURE_COUNT, Result->VAOs);
    glGenBuffers(HEX_FEATURE_COUNT, Result->VBOs);
    glGenBuffers(HEX_FEATURE_COUNT, Result->InstancedVBOs);

    material Materials[4 * (HEX_FEATURE_COUNT - 1)] = {0};

    for(int i = 1; i < HEX_FEATURE_COUNT; ++i) {
        char * ObjData = CrestLoadFileAsString(HexFeaturePaths[i]);
        Assert(ObjData != NULL);
        parsed_obj_format Obj = CrestParseOBJ(ObjData);
        Result->Features[i].MeshVertices = Obj.Mesh.VerticesCount;

        //Note(Zen): Convert material ID from ID offset in obj to offset from
        //all materials
        for(i32 j = 0; j < Obj.Mesh.VerticesCount; ++j) {
            Obj.Mesh.Vertices[j].MaterialID += 4 * (i-1);
        }
        for(i32 j = 0; j < 4; ++j) {
            Materials[4 * (i - 1) + j] = Obj.Materials[j];
        }
        free(ObjData);

        glBindVertexArray(Result->VAOs[i]);

        glBindBuffer(GL_ARRAY_BUFFER, Result->VBOs[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(obj_mesh), Obj.Mesh.Vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(obj_mesh_vertex), 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(obj_mesh_vertex), (void *)offsetof(obj_mesh_vertex, Normal));
        glEnableVertexAttribArray(1);
        glVertexAttribIPointer(2, 1, GL_INT, sizeof(obj_mesh_vertex), (void *)offsetof(obj_mesh_vertex, MaterialID));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, Result->InstancedVBOs[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(matrix) * MAX_FEATURE_SET_SIZE, &Result->Features[i].Model[0], GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(2+1);
        glVertexAttribPointer(2+1, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), 0);
        glEnableVertexAttribArray(3+1);
        glVertexAttribPointer(3+1, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void *)sizeof(v4));
        glEnableVertexAttribArray(4+1);
        glVertexAttribPointer(4+1, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void *)(2*sizeof(v4)));
        glEnableVertexAttribArray(5+1);
        glVertexAttribPointer(5+1, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void *)(3*sizeof(v4)));

        glVertexAttribDivisor(2+1, 1);
        glVertexAttribDivisor(3+1, 1);
        glVertexAttribDivisor(4+1, 1);
        glVertexAttribDivisor(5+1, 1);

    }
    Result->Shader = &App->Shaders.Feature;

    for(i32 i = 0; i < (HEX_FEATURE_COUNT - 1) * 4; ++i) {
        char UniformName[32];
        sprintf(UniformName, "Material[%d].Ambient", i);
        CrestShaderSetV3(Result->Shader->ShaderID, UniformName, Materials[i].Ambient);
        sprintf(UniformName, "Material[%d].Diffuse", i);
        CrestShaderSetV3(Result->Shader->ShaderID, UniformName, Materials[i].Diffuse);
        sprintf(UniformName, "Material[%d].Specular", i);
        CrestShaderSetV3(Result->Shader->ShaderID, UniformName, Materials[i].Specular);
        sprintf(UniformName, "Material[%d].Shininess", i);
        CrestShaderSetFloat(Result->Shader->ShaderID, UniformName, Materials[i].Shininess);
    }

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

internal matrix
GetFeatureTransform(hex_cell * Cell, hex_direction Direction) {
    v3 ModelPosition = CrestV3Add(Cell->Position, CrestV3Scale(HexCorners[Direction], HEX_SOLID_FACTOR * 0.5f));
    ModelPosition = NudgeFeature(ModelPosition);
    r32 ModelRotation = PI + (Direction * PI/3.f);//(RandomNoise3D(ModelPosition) * 2 - 1) * PI;
    r32 ModelScale = 0.9f + 0.2f * RandomNoise3D(CrestV3Scale(ModelPosition, 2.f * FEATURES_NOISE_SCALE));
    matrix Model = CrestM4MultM4(CrestMatrixRotation(ModelRotation, CREST_AXIS_Y), CrestMatrixScale(ModelScale));
    Model = CrestM4MultM4(CrestMatrixTranslation(ModelPosition), Model);
    return CrestMatrixTranspose(Model);
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
        Cell->Features[Direction] = 1;

        Set->Features[Type].Model[Cell->Index * HEX_DIRECTION_COUNT + Direction] = GetFeatureTransform(Cell, Direction);
    }

    glBindVertexArray(Set->VAOs[Type]);
    glBindBuffer(GL_ARRAY_BUFFER, Set->InstancedVBOs[Type]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(matrix) * MAX_FEATURE_SET_SIZE, &Set->Features[Type].Model[0]);

}

internal void
AddFeaturesToCellMask(hex_feature_set * Set, hex_cell * Cell, hex_feature_type Type, i32 Mask) {
    i32 Density = 0;
    Cell->FeatureType = Type;

    for(hex_direction Direction = 0; Direction < HEX_DIRECTION_COUNT; ++Direction) {
        if(Mask & (1<<Direction)) {
            Cell->Features[Direction] = 1;
            Set->Features[Type].Model[Cell->Index * HEX_DIRECTION_COUNT + Direction] = GetFeatureTransform(Cell, Direction);
            Density += 1;
        }
    }
    Cell->FeatureDensity = Density;
    glBindVertexArray(Set->VAOs[Type]);
    glBindBuffer(GL_ARRAY_BUFFER, Set->InstancedVBOs[Type]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(matrix) * MAX_FEATURE_SET_SIZE, &Set->Features[Type].Model[0]);
}

internal void
ClearFeaturesFromCell(hex_feature_set * Set, hex_cell * Cell) {
    for(hex_direction Direction = 0; Direction < HEX_DIRECTION_COUNT; ++Direction) {
        hex_feature_type Type = Cell->FeatureType;
        Set->Features[Type].Model[Cell->Index * HEX_DIRECTION_COUNT + Direction] = CrestMatrixZero();
        Cell->Features[Direction] = 0;

    }
    for(hex_feature_type Type = 1; Type < HEX_FEATURE_COUNT; ++Type) {
        glBindVertexArray(Set->VAOs[Type]);
        glBindBuffer(GL_ARRAY_BUFFER, Set->InstancedVBOs[Type]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(matrix) * MAX_FEATURE_SET_SIZE, &Set->Features[Type].Model[0]);
    }

    Cell->FeatureDensity = 0;
    Cell->FeatureType = 0;
}



internal void
DrawFeatureSet(hex_feature_set * FeatureSet) {
    glUseProgram(FeatureSet->Shader->ShaderID);
    for(i32 i = 1; i < HEX_FEATURE_COUNT; ++i) {
        glBindVertexArray(FeatureSet->VAOs[i]);
        glDrawArraysInstanced(GL_TRIANGLES, 0, FeatureSet->Features[i].MeshVertices, MAX_FEATURE_SET_SIZE);
    }
}
