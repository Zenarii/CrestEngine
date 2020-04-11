#define CREST_MESH_MAX_VERTICES 128

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
GetNextWord(char * WordStart) {
    char * WordEnd = WordStart;
    while(*WordEnd != ' ' && *WordEnd != '\n') ++WordEnd;
    return WordEnd;
}

internal mesh
CrestParseOBJ(char * Data) {
    mesh Result = {0};
    u32 VerticesCount = 0;

    v3 TempPositions[CREST_MESH_MAX_VERTICES];
    u32 PositionsCount = 0;
    v3 TempNormals[CREST_MESH_MAX_VERTICES];
    u32 NormalsCount = 0;
    v2 TempTexCoords[CREST_MESH_MAX_VERTICES];
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
            //Add to Normal
        }
        else if(CrestStringCompare(WordStart, "vt", WordLength) && WordLength == 2) {
            //Add to texture Coords
        }
        else if(CrestStringCompare(WordStart, "f", WordLength) && WordLength == 1) {
            //for now only works with triangles and only reads the position
            for(i32 i = 0; i < 3; ++i) {
                mesh_vertex Vertex = {0};
                WordStart = ++WordEnd;
                WordEnd = GetNextWord(WordStart);
                i32 VertexIndex = atoi(WordStart);
                Vertex.Position = TempPositions[VertexIndex-1];
                Result.Vertices[VerticesCount++] = Vertex;
            }
        }
        else {
            OutputDebugStringA("Failed to read line: ");
            //OutputDebugStringA(LineStart);
        }

        LineStart = ++LineEnd;
    }

    return Result;
}
