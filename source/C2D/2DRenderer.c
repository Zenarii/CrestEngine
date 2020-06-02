#define C2D_MAX_VERTICES 1024 * 6
#define C2D_MAX_TEXTURES 16

typedef struct C2DVertex  {
    v3 Position;
    v3 Colour;
    v2 TextureCoord;
    r32 TextureID;
} C2DVertex;

internal C2DVertex
C2DVertexInit(v3 Position, v3 Colour, v2 TextureCoord, i32 TextureID) {
    C2DVertex res = {Position, Colour, TextureCoord, TextureID};
    return res;
}
#define C2DVertex(pos, col, tcoord, tid) C2DVertexInit(pos, col, tcoord, tid)

typedef struct C2DRenderer {
    r32 Width;
    r32 Height;
    i32 DrawCalls;

    u32 VAO, VBO;
    u32 Shader;
    u32 BufferIndex;
    C2DVertex Vertices[C2D_MAX_VERTICES];

    i32 ActiveTextures;
    u32 Textures[C2D_MAX_TEXTURES];
} C2DRenderer;


internal void
C2DInit(C2DRenderer * Renderer) {
    //TODO(Zen): Must move these files
    Renderer->Shader = CrestLoadShader("../assets/vertex_shader.vs",
                                       "../assets/fragment_shader.fs", 0);

    {
        glGenVertexArrays(1, &Renderer->VAO);
        glBindVertexArray(Renderer->VAO);

        glGenBuffers(1, &Renderer->VBO);
        glBindBuffer(GL_ARRAY_BUFFER, Renderer->VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Renderer->Vertices), Renderer->Vertices, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(C2DVertex), 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(C2DVertex), (void*)offsetof(C2DVertex, Colour));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(C2DVertex), (void*)offsetof(C2DVertex, TextureCoord));
        glEnableVertexAttribArray(2);

        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(C2DVertex), (void*)offsetof(C2DVertex, TextureID));
        glEnableVertexAttribArray(3);
    }

    {
        glUseProgram(Renderer->Shader);
        i32 Location = glGetUniformLocation(Renderer->Shader, "Images");
        int samplers[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
        glUniform1iv(Location, 16, samplers);
    }

    Renderer->BufferIndex = 0;
    Renderer->DrawCalls = 0;
}

internal void
C2DFlush(C2DRenderer * Renderer) {
    for(i32 i = 0; i < Renderer->ActiveTextures; ++i) {
        glBindTextureUnit(i, Renderer->Textures[i]);
    }

    glUseProgram(Renderer->Shader);
    CrestShaderSetFloat(Renderer->Shader, "RendererWidth", Renderer->Width);
    CrestShaderSetFloat(Renderer->Shader, "RendererHeight", Renderer->Height);
    glBindVertexArray(Renderer->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Renderer->VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, Renderer->BufferIndex * sizeof(C2DVertex), Renderer->Vertices);
    glDrawArrays(GL_TRIANGLES, 0, Renderer->BufferIndex);

    Renderer->BufferIndex = 0;
    Renderer->DrawCalls++;
}


internal i32
C2DEndFrame(C2DRenderer * Renderer) {
    C2DFlush(Renderer);
    i32 DrawCalls = Renderer->DrawCalls;
    Renderer->DrawCalls = 0;
    return DrawCalls;
}

//Note(Zen): TextureID of white square should always be 0
internal void
C2DDrawColouredRect(C2DRenderer * Renderer, v2 Position, v2 Size, v3 Colour) {
    if(Renderer->BufferIndex + 6 > C2D_MAX_VERTICES) C2DFlush(Renderer);

    //Note(Zen): First Triangle
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x, Position.y, 0.0f), Colour, v2(0.1f, 0.1f), 0);
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x + Size.x, Position.y, 0.0f), Colour, v2(0.9f, 0.1f), 0);
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x, Position.y + Size.y, 0.0f), Colour, v2(0.1f, 0.9f), 0);

    //Note(Zen): Second Tangle
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x + Size.x, Position.y + Size.y, 0.0f), Colour, v2(0.9f, 0.9f), 0);
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x + Size.x, Position.y, 0.0f), Colour, v2(0.9f, 0.1f), 0);
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x, Position.y + Size.y, 0.0f), Colour, v2(0.1f, 0.9f), 0);
}

internal void
C2DDrawTexturedRectTint(C2DRenderer * Renderer, v2 Position, v2 Size, i32 TextureID, v3 Colour) {
    if(Renderer->BufferIndex + 6 > C2D_MAX_VERTICES) C2DFlush(Renderer);

    //Note(Zen): First Triangle
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x, Position.y, 0.0f), Colour, v2(0.0f, 0.0f), TextureID);
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x + Size.x, Position.y, 0.0f), Colour, v2(1.0f, 0.0f), TextureID);
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x, Position.y + Size.y, 0.0f), Colour, v2(0.0f, 1.0f), TextureID);

    //Note(Zen): Second Tangle
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x + Size.x, Position.y + Size.y, 0.0f), Colour, v2(1.0f, 1.0f), TextureID);
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x + Size.x, Position.y, 0.0f), Colour, v2(1.0f, 0.0f), TextureID);
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x, Position.y + Size.y, 0.0f), Colour, v2(0.0f, 1.0f), TextureID);
}

internal void
C2DDrawTexturedRect(C2DRenderer * Renderer, v2 Position, v2 Size, i32 TextureID) {
    const v3 Colour = v3(1.0f, 1.0f, 1.0f);
    C2DDrawTexturedRectTint(Renderer, Position, Size, TextureID, Colour);
}

internal void
C2DDrawTexturedSlice(C2DRenderer * Renderer, v2 Position, v2 Size, v4 Rect, i32 TextureID) {
    if(Renderer->BufferIndex + 6 > C2D_MAX_VERTICES) C2DFlush(Renderer);

    const v3 Colour = v3(1.0f, 1.0f, 1.0f);
    //Note(Zen): First Triangle
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x, Position.y, 0.0f), Colour, v2(Rect.x, Rect.y), TextureID);
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x + Size.x, Position.y, 0.0f), Colour, v2(Rect.x + Rect.width, Rect.y), TextureID);
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x, Position.y + Size.y, 0.0f), Colour, v2(Rect.x, Rect.y + Rect.height), TextureID);

    //Note(Zen): Second Tangle
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x + Size.x, Position.y + Size.y, 0.0f), Colour, v2(Rect.x + Rect.width, Rect.y + Rect.height), TextureID);
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x + Size.x, Position.y, 0.0f), Colour, v2(Rect.x + Rect.width, Rect.y), TextureID);
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x, Position.y + Size.y, 0.0f), Colour, v2(Rect.x, Rect.y + Rect.height), TextureID);
}
