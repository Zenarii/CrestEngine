#define CREST_MESH_MAX_VERTICES 1024
/*
    Mesh that will be returned.
*/
typedef struct mesh_vertex mesh_vertex;
struct mesh_vertex {
    v3 Position;
    v3 Normal;
    v3 Colour;
    v2 TextureCoord;
    r32 TextureID;
};

typedef struct mesh mesh;
struct mesh {
    u32 VerticesCount;
    mesh_vertex Vertices[CREST_MESH_MAX_VERTICES];
};


/*
    OBJ
*/
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

internal mesh
CrestParseOBJ(char * Data) {
    mesh Result = {0};
    u32 VerticesCount = 0;

    v3 TempPositions[CREST_MESH_MAX_VERTICES] = {0};
    u32 PositionsCount = 0;
    v3 TempNormals[CREST_MESH_MAX_VERTICES] = {0};
    u32 NormalsCount = 0;
    v2 TempTexCoords[CREST_MESH_MAX_VERTICES] = {0};
    u32 TexCoordsCount = 0;

    char * LineStart = Data;
    char * LineEnd = Data;

    while(*LineEnd != '\0') {

        //Note(Zen): Get line
        while (*LineEnd != '\n' && *LineEnd != '\0') ++LineEnd;

        char * WordStart = LineStart;
        char * WordEnd = LineStart;
        while(*WordEnd != ' ') ++WordEnd;

        i32 WordLength = WordEnd - WordStart;

        if(CrestStringCompare(WordStart, "#", WordLength) && WordLength == 1) {
            //skip bc comment
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
            //Add to texture Coords
        }
        else if(CrestStringCompare(WordStart, "f", WordLength) && WordLength == 1) {
            //for now only works with triangles
            for(i32 i = 0; i < 3; ++i) {
                mesh_vertex Vertex = {0};

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


                Result.Vertices[VerticesCount++] = Vertex;

                if(!i) {
                    Result.Vertices[VerticesCount].Position = v3(0.f, 0.f, 0.f);
                }
            }
        }
        else {
            OutputDebugStringA("Failed to read line: ");
            //OutputDebugStringA(LineStart);
        }

        LineStart = ++LineEnd;
    }
    Assert(VerticesCount < CREST_MESH_MAX_VERTICES);
    Assert(NormalsCount < CREST_MESH_MAX_VERTICES);
    Assert(TexCoordsCount < CREST_MESH_MAX_VERTICES);
    Result.VerticesCount = VerticesCount;
    return Result;
}
