#include "../CrestMaths.cpp"
#include "string.h"

typedef struct vertex {
    vector3 Position;
    vector3 Normal;
    vector2 TexCoord;
} vertex;

typedef struct mesh {
    vertex Vertices[1024];
    int VerticesCount;
    CrestTexture Texture;
    unsigned int VAO, VBO;

} mesh;



internal char *
CrestGetCharacterPosition(char * String, char Character) {
    int Point = 0;
    for(;;) {
        if(String[Point] == 0) {
            return 0;
        }
        if(String[Point] == Character) {
            return String+Point;
        }
        else {
            ++Point;
        }
    }
}

internal int
CrestGetStringLength(const char * String) {
    int Length = 0;
    while (String[Length]) {
        ++Length;
    }
    return Length;
}

internal bool
CrestCompareStrings(const char * s1, const char * s2, int count) {
    for(int i = 0; i < count; ++i) {
        if(s1[i] != s2[i]) {
            return false;
        }
    }
    return true;
}

#define CREST_LOAD_MESH_MAX_VERTICES 1024

//Note(Zen): Only works for objs atm
internal mesh
CrestLoadMesh(const char * Path) {

    vector3 TempVertices[CREST_LOAD_MESH_MAX_VERTICES] = {};
    vector3 TempNormals[CREST_LOAD_MESH_MAX_VERTICES] = {};
    vector2 TempTextures[CREST_LOAD_MESH_MAX_VERTICES] = {};
    int UsedVertices = 0;
    int UsedNormals = 0;
    int UsedTextures = 0;

    mesh Mesh = {};

    vertex FinalVertices[CREST_LOAD_MESH_MAX_VERTICES] = {};
    int FinalVerticesCount = 0;

    char * MeshSource = CrestLoadFileAsString(Path);
    char * CurrentLine = MeshSource;
    while(CurrentLine) {
        //Note(Zen): Split file into lines
        char * NextLine = CrestGetCharacterPosition(CurrentLine, '\n');
        int CurrentLineLength = NextLine ? (NextLine-CurrentLine) : CrestGetStringLength(CurrentLine);

        char * TempString = (char*)malloc(CurrentLineLength+1);
        if (TempString) {
            memcpy(TempString, CurrentLine, CurrentLineLength);
            TempString[CurrentLineLength] = '\0';

            //Note(Zen): Split the TempString into chunks
            const int MAX_CHUNKS_IN_OBJ = 16;
            char *Chunk[MAX_CHUNKS_IN_OBJ] = {};
            char * Token = strtok(TempString, " ");
            Chunk[0] = Token;
            int i = 1;
            while(Token) {
                Token = strtok(NULL, " ");
                Chunk[i] = Token;
                ++i;
            }

            //Note(Zen): Act on chunks
            if(CrestCompareStrings(Chunk[0], "#", 1)) {
                //Note(Zen): Skip Comments
            }
            else if(CrestCompareStrings(Chunk[0], "o", 1)) {
                CrestLog("Opening: ");
                CrestLog(Chunk[1]);
                CrestLog(" Mesh.\n");
            }
            else if(CrestCompareStrings(Chunk[0], "vn", 2)) {
                TempNormals[UsedNormals].x = strtof(Chunk[1], NULL);
                TempNormals[UsedNormals].y = strtof(Chunk[2], NULL);
                TempNormals[UsedNormals].z = strtof(Chunk[3], NULL);
                ++UsedNormals;
            }
            else if(CrestCompareStrings(Chunk[0], "vt", 2)) {
                TempTextures[UsedTextures].x = strtof(Chunk[1], NULL);
                TempTextures[UsedTextures].y = strtof(Chunk[2], NULL);
                ++UsedTextures;
            }
            else if(CrestCompareStrings(Chunk[0], "v", 1)) {
                TempVertices[UsedVertices].x = strtof(Chunk[1], NULL);
                TempVertices[UsedVertices].y = strtof(Chunk[2], NULL);
                TempVertices[UsedVertices].z = strtof(Chunk[3], NULL);
                ++UsedVertices;
            }
            else if(CrestCompareStrings(Chunk[0], "f", 1)) {
                //Note(Zen): For now only works with triangles, and has no indexing
                for (int j = 1; j < 4; j++) {
                    char * FaceChunk[3] = {};
                    //Split each chunk into a smaller sub chunk
                    i = 1;
                    Token = strtok(Chunk[j], "/");
                    FaceChunk[0] = Token;
                    while(Token && (i < 3)) {
                        Token = strtok(NULL, "/");
                        FaceChunk[i] = Token;
                        ++i;
                    }
                    int VertexIndex = atoi(FaceChunk[0])-1;
                    FinalVertices[FinalVerticesCount].Position = TempVertices[VertexIndex];
                    int TextureIndex = atoi(FaceChunk[1])-1;
                    FinalVertices[FinalVerticesCount].TexCoord = TempTextures[TextureIndex];
                    int NormalIndex = atoi(FaceChunk[2])-1;
                    FinalVertices[FinalVerticesCount].Normal = TempNormals[NormalIndex];
                    ++FinalVerticesCount;
                }
            }
            else {
                CrestLog("Failed to read instruction: '");
                CrestLog(Chunk[0]);
                CrestLog("' when loading .obj\n");
            }

            free(TempString);
        }
        else {
            CrestLog("Malloc Failed while loading mesh");
        }

        CurrentLine = NextLine ? (NextLine+1) : NULL;
        int i = 0;
    }
    //Note(Zen): transfer vertices from file load and generate buffers
    memcpy(Mesh.Vertices, FinalVertices, FinalVerticesCount*sizeof(vertex));
    Mesh.VerticesCount = FinalVerticesCount;

    glGenVertexArrays(1, &Mesh.VAO);
    glBindVertexArray(Mesh.VAO);

    glGenBuffers(1, &Mesh.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, Mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex)*FinalVerticesCount, &Mesh.Vertices, GL_STATIC_DRAW);

    Mesh.Texture = CrestTextureInit("../data/woodbox.png");

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, Normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, TexCoord));


    free(MeshSource);
    return Mesh;
}

internal void
CrestRenderMesh(mesh * MeshToDraw, CrestShader shaderProgram, platform * Platform) {

    
    glBindTexture(GL_TEXTURE_2D, MeshToDraw->Texture);
    glBindVertexArray(MeshToDraw->VAO);
    glUseProgram(shaderProgram);

    const real32 radius = 10.0f;
    real32 camX = sinf(Platform->TotalTime * 0.001f) * radius;
    real32 camZ = cosf(Platform->TotalTime * 0.001f) * radius;
    real32 Ratio = ((real32)Platform->ScreenWidth)/((real32)Platform->ScreenHeight);
    matrix4 Projection = CrestProjectionMatrix(PI/2.0f, Ratio, 0.1f, 100.0f);
    matrix4 View = CrestLookAt(Vector3(camX, 0.0f, camZ), Vector3(0.0f, 0.0f, 0.0f));


    CrestShaderSetM4(shaderProgram, "Projection", &Projection);
    CrestShaderSetM4(shaderProgram, "View", &View);
    matrix4 Model = Matrix4(1.0f);
    CrestShaderSetM4(shaderProgram, "Model", &Model);

    glDrawArrays(GL_TRIANGLES, 0, MeshToDraw->VerticesCount);
    glBindVertexArray(0);
}
