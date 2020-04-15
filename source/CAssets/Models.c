#define CREST_MESH_MAX_VERTICES 1024



/*
    OBJ
*/

typedef struct obj_mesh_vertex obj_mesh_vertex;
struct obj_mesh_vertex {
    v3 Position;
    v3 Normal;
    v2 TextureCoord;
    i32 MaterialID;
};

typedef struct obj_mesh obj_mesh;
struct obj_mesh {
    u32 VerticesCount;
    obj_mesh_vertex Vertices[CREST_MESH_MAX_VERTICES];
};

typedef struct material material;
struct material {
    v3 Ambient;
    v3 Diffuse;
    v3 Specular;
    r32 Shininess;
};

typedef struct parsed_obj_format parsed_obj_format;
struct parsed_obj_format {
    obj_mesh Mesh;
    i32 UsedMaterials;
    material Materials[4];
};

typedef struct material_library material_library;
struct material_library {
    i32 MaterialsCount;
    struct {
        char Name[32]; //HARDCODE(Zen): Expect all material names length to be < 32
        material Material;
    } Materials[4];
};

internal b32
CrestStringCompare(char * String1, char * String2, i32 Length) {
    b32 Result = 1;
    for(i32 i = 0; i < Length; ++i) {
        if(String1[i] != String2[i]) Result = 0;
    }
    return Result;
}

internal char *
GetNextDelim(char * WordStart, char Delim) {
    char * WordEnd = WordStart;
    while(*WordEnd != Delim) ++WordEnd;
    return WordEnd;
}

internal char *
GetNextWord(char * WordStart) {
    char * WordEnd = WordStart;
    while(*WordEnd != ' ' && *WordEnd != '\n') ++WordEnd;
    return WordEnd;
}

internal material_library
CrestLoadAndParseMTLlib(char * FileName, i32 FileNameLength) {
    material_library Result = {0};
    Result.MaterialsCount = -1;
    //HARDCODE(Zen): Path to the models
    char FilePath[64] = "../assets/FeatureModels/";
    strncat(FilePath, FileName, FileNameLength);
    char * Data = CrestLoadFileAsString(FilePath);

    char * LineStart = Data;
    char * LineEnd = Data;

    i32 LineNum = 0;

    while(*LineEnd != '\0') {
        LineNum++;

        while (*LineEnd != '\n' && *LineEnd != '\0') ++LineEnd;

        char * WordStart = LineStart;
        char * WordEnd = LineStart;
        while(*WordEnd != ' ') ++WordEnd;

        i32 WordLength = WordEnd - WordStart;

        if(CrestStringCompare(WordStart, "#", WordLength) && WordLength == 1) {
            //skip comment
        }
        else if(CrestStringCompare(WordStart, "newmtl", WordLength) && WordLength == 6) {
            WordStart = ++WordEnd;
            WordEnd = GetNextWord(WordStart);

            char Buffer [64] = "(MTL) Loading material: ";
            strncat(Buffer, WordStart, WordEnd-WordStart);
            strcat(Buffer, "\n");
            OutputDebugStringA(Buffer);

            i32 Count = ++Result.MaterialsCount;
            //UNTESTED(Zen):
            if(Count >= 4) {
                OutputDebugStringA("(MTL) Loaded too many materials! Overwriting the first material.\n");
                Count = 0;
            }
            strncpy(Result.Materials[Count].Name, WordStart, WordEnd-WordStart);
        }
        else if(CrestStringCompare(WordStart, "Ns", WordLength) && WordLength == 2) {
            i32 Count = Result.MaterialsCount;

            WordStart = ++WordEnd;
            WordEnd = GetNextWord(WordStart);

            Result.Materials[Count].Material.Shininess = atof(WordStart);
        }
        else if(CrestStringCompare(WordStart, "Ka", WordLength) && WordLength == 2) {
            i32 Count = Result.MaterialsCount;

            WordStart = ++WordEnd;
            WordEnd = GetNextWord(WordStart);

            Result.Materials[Count].Material.Ambient.x = atof(WordStart);

            WordStart = ++WordEnd;
            WordEnd = GetNextWord(WordStart);

            Result.Materials[Count].Material.Ambient.y = atof(WordStart);

            WordStart = ++WordEnd;
            WordEnd = GetNextWord(WordStart);

            Result.Materials[Count].Material.Ambient.z = atof(WordStart);
        }
        else if(CrestStringCompare(WordStart, "Kd", WordLength) && WordLength == 2) {
            i32 Count = Result.MaterialsCount;

            WordStart = ++WordEnd;
            WordEnd = GetNextWord(WordStart);

            Result.Materials[Count].Material.Diffuse.x = atof(WordStart);

            WordStart = ++WordEnd;
            WordEnd = GetNextWord(WordStart);

            Result.Materials[Count].Material.Diffuse.y = atof(WordStart);

            WordStart = ++WordEnd;
            WordEnd = GetNextWord(WordStart);

            Result.Materials[Count].Material.Diffuse.z = atof(WordStart);
        }
        else if(CrestStringCompare(WordStart, "Ks", WordLength) && WordLength == 2) {
            i32 Count = Result.MaterialsCount;

            WordStart = ++WordEnd;
            WordEnd = GetNextWord(WordStart);

            Result.Materials[Count].Material.Specular.x = atof(WordStart);

            WordStart = ++WordEnd;
            WordEnd = GetNextWord(WordStart);

            Result.Materials[Count].Material.Specular.y = atof(WordStart);

            WordStart = ++WordEnd;
            WordEnd = GetNextWord(WordStart);

            Result.Materials[Count].Material.Specular.z = atof(WordStart);
        }
        //Note(Zen) No support for transparent materials
        //but don't want to see that in Debug Output
        else if(CrestStringCompare(WordStart, "Ni", WordLength) || CrestStringCompare(WordStart, "d", WordLength)) {}
        else {
            char Out[64];
            sprintf(Out, "(MTL) Failed to read line: %d\n", LineNum);
            OutputDebugStringA(Out);
        }


        LineStart = ++LineEnd;
    }

    return Result;
}


internal parsed_obj_format
CrestParseOBJ(char * Data) {
    obj_mesh Mesh = {0};
    material_library MaterialLibrary = {0};
    u32 VerticesCount = 0;
    i32 CurrentMaterialID;

    v3 TempPositions[CREST_MESH_MAX_VERTICES] = {0};
    u32 PositionsCount = 0;
    v3 TempNormals[CREST_MESH_MAX_VERTICES] = {0};
    u32 NormalsCount = 0;
    v2 TempTexCoords[CREST_MESH_MAX_VERTICES] = {0};
    u32 TexCoordsCount = 0;



    char * LineStart = Data;
    char * LineEnd = Data;

    i32 LineNum = 0;

    while(*LineEnd != '\0') {
        LineNum++;
        //Note(Zen): Get line
        while (*LineEnd != '\n' && *LineEnd != '\0') ++LineEnd;

        char * WordStart = LineStart;
        char * WordEnd = LineStart;
        while(*WordEnd != ' ') ++WordEnd;

        i32 WordLength = WordEnd - WordStart;

        if(CrestStringCompare(WordStart, "#", WordLength) && WordLength == 1) {
            //skip bc comment
        }
        else if(CrestStringCompare(WordStart, "o", WordLength) && WordLength == 1) {
            WordStart = ++WordEnd;
            WordEnd = GetNextWord(WordStart);

            char Buffer [32] = "(OBJ) Loading object: ";
            strncat(Buffer, WordStart, WordEnd-WordStart);
            strcat(Buffer, "\n");
            OutputDebugStringA(Buffer);
        }
        else if(CrestStringCompare(WordStart, "mtllib", WordLength) && WordLength == 6) {
            WordStart = ++WordEnd;
            WordEnd = GetNextWord(WordStart);

            char Buffer [32] = "(OBJ) Loading mtllib: ";
            strncat(Buffer, WordStart, WordEnd-WordStart);
            strcat(Buffer, "\n");
            OutputDebugStringA(Buffer);

            MaterialLibrary = CrestLoadAndParseMTLlib(WordStart, WordEnd-WordStart);
        }
        else if(CrestStringCompare(WordStart, "usemtl", WordLength) && WordLength == 6) {
            WordStart = ++WordEnd;
            WordEnd = GetNextWord(WordStart);
            //get ID


            for(i32 i = 0; i < MaterialLibrary.MaterialsCount + 1; ++i) {
                if(CrestStringCompare(WordStart, MaterialLibrary.Materials[i].Name, WordEnd-WordStart)) {
                    CurrentMaterialID = i;
                    char Buffer[32];
                    sprintf(Buffer, "(OBJ) using %dth material\n", i);
                    OutputDebugStringA(Buffer);
                }
            }
        }
        else if(CrestStringCompare(WordStart, "v", WordLength) && WordLength == 1) {
            v3 Position = {0};
            //add to Vertices
            WordStart = ++WordEnd;
            WordEnd = GetNextWord(WordStart);
            Position.x = atof(WordStart);

            WordStart = ++WordEnd;
            WordEnd = GetNextWord(WordStart);

            Position.y = atof(WordStart);


            WordStart = ++WordEnd;
            WordEnd = GetNextWord(WordStart);

            Position.z = atof(WordStart);

            TempPositions[PositionsCount++] = Position;
        }
        else if(CrestStringCompare(WordStart, "vn", WordLength) && WordLength == 2) {
            v3 Normal = {0};
            //add to Vertices
            WordStart = ++WordEnd;
            WordEnd = GetNextWord(WordStart);
            Normal.x = atof(WordStart);

            WordStart = ++WordEnd;
            WordEnd = GetNextWord(WordStart);

            Normal.y = atof(WordStart);


            WordStart = ++WordEnd;
            WordEnd = GetNextWord(WordStart);

            Normal.z = atof(WordStart);

            TempNormals[NormalsCount++] = Normal;
        }
        else if(CrestStringCompare(WordStart, "vt", WordLength) && WordLength == 2) {
            v2 Uv = {0};
            WordStart = ++WordEnd;
            WordEnd = GetNextWord(WordStart);

            Uv.x = atof(WordStart);


            WordStart = ++WordEnd;
            WordEnd = GetNextWord(WordStart);

            Uv.y = atof(WordStart);

            TempTexCoords[TexCoordsCount++] = Uv;
        }
        else if(CrestStringCompare(WordStart, "f", WordLength) && WordLength == 1) {
            //for now only works with triangles
            for(i32 i = 0; i < 3; ++i) {
                obj_mesh_vertex Vertex = {0};

                WordStart = ++WordEnd;
                WordEnd = GetNextDelim(WordStart, '/');
                i32 PositionIndex = strtol(WordStart, &WordEnd, 10);
                Vertex.Position = TempPositions[PositionIndex - 1];


                WordStart = ++WordEnd;
                WordEnd = GetNextDelim(WordStart, '/');
                //Note(Zen) If not moved forwards there was no texture coordinate
                if(WordStart != WordEnd) {
                    //add texture coordinate
                }


                WordStart = ++WordEnd;
                WordEnd = GetNextWord(WordStart);
                i32 NormalIndex = strtol(WordStart, &WordEnd, 10);
                Vertex.Normal = TempNormals[NormalIndex - 1];
                Vertex.MaterialID = CurrentMaterialID;

                Mesh.Vertices[VerticesCount++] = Vertex;
            }
        }
        else {
            char Out[64] = {0};
            sprintf(Out, "(OBJ) Failed to read line: %d\n", LineNum);
            OutputDebugStringA(Out);
        }

        LineStart = ++LineEnd;
    }
    LineStart = --LineEnd;
    Assert(VerticesCount < CREST_MESH_MAX_VERTICES);
    Assert(NormalsCount < CREST_MESH_MAX_VERTICES);
    Assert(TexCoordsCount < CREST_MESH_MAX_VERTICES);
    Mesh.VerticesCount = VerticesCount;

    parsed_obj_format Result = {0};
    Result.Mesh = Mesh;
    for(i32 i = 0; i < 4; ++i) {
        Result.Materials[i] = MaterialLibrary.Materials[i].Material;
    }
    return Result;
}
