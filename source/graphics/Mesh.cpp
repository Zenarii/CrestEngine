#include "../CrestMaths.cpp"
#include "string.h"

typedef struct vertex {
    vector3 Position;
    vector3 Normal;
    vector2 TexCoord;
} vertex;

typedef struct mesh {
    vertex Vertices[1000];
    unsigned int Indices[500];


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

//Note(Zen): Only works for objs atm
internal void
CrestLoadMesh(const char * Path) {

    vector3 TempVertices[16];
    vector3 TempNormals[16];
    vector2 TempTextures[16];
    int UsedVertices = 0;
    int UsedNormals = 0;
    int UsedTextures = 0;

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
            char *Chunk[MAX_CHUNKS_IN_OBJ];
            char * Token = strtok(TempString, " ");
            Chunk[0] = Token;
            int i = 1;
            while(Token) {
                Token = strtok(NULL, " ");
                Chunk[i] = Token;
                ++i;
            }
            i = 27;
            //Note(Zen): Act on chunks
            if(CrestCompareStrings(Chunk[0], "#", 1)) {

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
                CrestLog("f");
            }
            free(TempString);
        }
        else {
            CrestLog("Malloc Failed while loading mesh");
        }

        CurrentLine = NextLine ? (NextLine+1) : NULL;
        int i = 0;
    }

    free(MeshSource);
}
