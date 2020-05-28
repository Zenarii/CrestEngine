#define C3D_MAX_VERTICES 1024
#define C3D_MAX_INDICIES 526
#define C3D_MAX_TEXTURES 16

typedef struct C3DVertex {
    v3 Position;
    v3 Colour;
    v2 TextureCoord;
    r32 TextureID;
} C3DVertex;

internal C3DVertex
C3DVertexInit(v3 Position, v3 Colour, v2 TextureCoord, r32 TextureID) {
    C3DVertex Result = {Position, Colour, TextureCoord, TextureID};
    return Result;
}
#define C3DVertex(Position, Colour, TextureCoord, TextureID) C3DVertexInit(Position, Colour, TextureCoord, TextureID)

typedef struct C3DRenderer C3DRenderer;
struct C3DRenderer {
    u32 VAO, VBO, EBO;
    u32 Shader;
    u32 BufferIndex;
    C3DVertex Vertices[C3D_MAX_VERTICES];
    u32 IndicesIndex;
    u32 Indices[C3D_MAX_INDICIES];


    i32 ActiveTextures;
    u32 Textures[C3D_MAX_TEXTURES];

};

internal void
C3DInit(C3DRenderer * Renderer) {
    Renderer->Shader = CrestLoadShader("assets/3D_vertex_shader.vs",
                                       "assets/3D_fragment_shader.fs");

    {
        glGenVertexArrays(1, &Renderer->VAO);
        glBindVertexArray(Renderer->VAO);


        glGenBuffers(1, &Renderer->VBO);
        glBindBuffer(GL_ARRAY_BUFFER, Renderer->VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Renderer->Vertices), Renderer->Vertices, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &Renderer->EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Renderer->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Renderer->Indices), Renderer->Indices, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(C3DVertex), 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(C3DVertex), (void *)offsetof(C3DVertex, Colour));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(C3DVertex), (void *)offsetof(C3DVertex, TextureCoord));
        glEnableVertexAttribArray(2);

        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(C3DVertex), (void *)offsetof(C3DVertex, TextureID));
        glEnableVertexAttribArray(3);
    }

    //Note(Zen): Set up Texture Array
    {
        glUseProgram(Renderer->Shader);
        i32 Location = glGetUniformLocation(Renderer->Shader, "Images");
        int samplers[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
        glUniform1iv(Location, 16, samplers);
    }

    Renderer->BufferIndex = 0;
    Renderer->IndicesIndex = 0;
}

internal void
C3DFlush(C3DRenderer * Renderer) {
    //Note(Zen): Set Active Textures
    for(i32 i = 0; i < Renderer->ActiveTextures; ++i) {
        glBindTextureUnit(i, Renderer->Textures[i]);
    }

    glUseProgram(Renderer->Shader);
    glBindVertexArray(Renderer->VAO);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Renderer->EBO);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, Renderer->IndicesIndex * sizeof(u32), Renderer->Indices);

    glBindBuffer(GL_ARRAY_BUFFER, Renderer->VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, Renderer->BufferIndex * sizeof(C3DVertex), Renderer->Vertices);

    glDrawElements(GL_TRIANGLES, Renderer->IndicesIndex, GL_UNSIGNED_INT, 0);

    Renderer->IndicesIndex = 0;
    Renderer->BufferIndex = 0;
}

internal void
C3DDrawQuad(C3DRenderer * Renderer, v3 p0, v3 p1, v3 p2, v3 p3, v3 Colour) {
    if(Renderer->BufferIndex + 4 > C3D_MAX_VERTICES || Renderer->IndicesIndex + 6 > C3D_MAX_INDICIES) C3DFlush(Renderer);

    i32 StartIndex = Renderer->BufferIndex;

    Renderer->Vertices[Renderer->BufferIndex++] = C3DVertex(p0, Colour, v2(0.1f, 0.1f), 0);
    Renderer->Vertices[Renderer->BufferIndex++] = C3DVertex(p1, Colour, v2(0.1f, 0.1f), 0);
    Renderer->Vertices[Renderer->BufferIndex++] = C3DVertex(p2, Colour, v2(0.1f, 0.1f), 0);
    Renderer->Vertices[Renderer->BufferIndex++] = C3DVertex(p3, Colour, v2(0.1f, 0.1f), 0);

    Renderer->Indices[Renderer->IndicesIndex] = StartIndex;
    Renderer->Indices[Renderer->IndicesIndex + 1] = StartIndex + 1;
    Renderer->Indices[Renderer->IndicesIndex + 2] = StartIndex + 2;
    Renderer->Indices[Renderer->IndicesIndex + 3] = StartIndex + 1;
    Renderer->Indices[Renderer->IndicesIndex + 4] = StartIndex + 2;
    Renderer->Indices[Renderer->IndicesIndex + 5] = StartIndex + 3;
    Renderer->IndicesIndex += 6;
}

internal void
C3DDrawTri(C3DRenderer * Renderer, v3 p0, v3 p1, v3 p2, v3 Colour) {
    if(Renderer->BufferIndex + 3 > C3D_MAX_VERTICES || Renderer->IndicesIndex + 3 > C3D_MAX_INDICIES) C3DFlush(Renderer);

    i32 StartIndex = Renderer->BufferIndex;

    Renderer->Vertices[Renderer->BufferIndex++] = C3DVertex(p0, Colour, v2(0.1f, 0.1f), 0);
    Renderer->Vertices[Renderer->BufferIndex++] = C3DVertex(p1, Colour, v2(0.1f, 0.1f), 0);
    Renderer->Vertices[Renderer->BufferIndex++] = C3DVertex(p2, Colour, v2(0.1f, 0.1f), 0);

    Renderer->Indices[Renderer->IndicesIndex] = StartIndex;
    Renderer->Indices[Renderer->IndicesIndex + 1] = StartIndex + 1;
    Renderer->Indices[Renderer->IndicesIndex + 2] = StartIndex + 2;
    Renderer->IndicesIndex += 3;
}

internal void
C3DDrawCube(C3DRenderer * Renderer, v3 Center, v3 Colour, r32 Scale) {
    if(Renderer->BufferIndex + 8 > C3D_MAX_VERTICES || Renderer->IndicesIndex + 36 > C3D_MAX_INDICIES) C3DFlush(Renderer);

    Renderer->Vertices[Renderer->BufferIndex++] = C3DVertex(CrestV3Add(Center, CrestV3Scale(v3(-1, -1, -1), Scale)), Colour, v2(0.1, 0.1f), 0);
    Renderer->Vertices[Renderer->BufferIndex++] = C3DVertex(CrestV3Add(Center, CrestV3Scale(v3( 1, -1, -1), Scale)), Colour, v2(0.1, 0.1f), 0);
    Renderer->Vertices[Renderer->BufferIndex++] = C3DVertex(CrestV3Add(Center, CrestV3Scale(v3( 1,  1, -1), Scale)), Colour, v2(0.1, 0.1f), 0);
    Renderer->Vertices[Renderer->BufferIndex++] = C3DVertex(CrestV3Add(Center, CrestV3Scale(v3(-1,  1, -1), Scale)), Colour, v2(0.1, 0.1f), 0);

    Renderer->Vertices[Renderer->BufferIndex++] = C3DVertex(CrestV3Add(Center, CrestV3Scale(v3(-1, -1,  1), Scale)), Colour, v2(0.1, 0.1f), 0);
    Renderer->Vertices[Renderer->BufferIndex++] = C3DVertex(CrestV3Add(Center, CrestV3Scale(v3( 1, -1,  1), Scale)), Colour, v2(0.1, 0.1f), 0);
    Renderer->Vertices[Renderer->BufferIndex++] = C3DVertex(CrestV3Add(Center, CrestV3Scale(v3( 1,  1,  1), Scale)), Colour, v2(0.1, 0.1f), 0);
    Renderer->Vertices[Renderer->BufferIndex++] = C3DVertex(CrestV3Add(Center, CrestV3Scale(v3(-1,  1,  1), Scale)), Colour, v2(0.1, 0.1f), 0);

    u32 StartIndices = Renderer->BufferIndex - 8;

    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 0;
    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 2;
    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 1;

    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 0;
    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 3;
    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 2;

    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 1;
    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 2;
    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 6;

    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 6;
    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 5;
    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 1;


    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 4;
    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 5;
    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 6;

    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 6;
    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 7;
    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 4;


    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 2;
    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 3;
    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 6;

    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 6;
    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 3;
    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 7;


    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 0;
    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 7;
    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 3;

    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 0;
    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 4;
    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 7;


    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 0;
    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 1;
    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 5;

    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 0;
    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 5;
    Renderer->Indices[Renderer->IndicesIndex++] = StartIndices + 4;
}
