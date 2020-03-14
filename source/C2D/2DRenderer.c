#define C2D_MAX_VERTICES 1024 * 6
#define C2D_MAX_TEXTURES 4

typedef struct C2DVertex  {
    v3 Position;
    v3 Colour;
    v2 TextureCoord;
} C2DVertex;

internal C2DVertex
C2DVertexInit(v3 Position, v3 Colour, v2 TextureCoord) {
    C2DVertex res = {Position, Colour, TextureCoord};
    return res;
}
#define C2DVertex(pos, col, tcoord) C2DVertexInit(pos, col, tcoord)

typedef struct C2DRenderer {
    r32 Width;
    r32 Height;
    i32 DrawCalls;

    u32 VAO, VBO;
    u32 Shader;
    u32 BufferIndex;
    C2DVertex Vertices[C2D_MAX_VERTICES];

    u32 Textures[C2D_MAX_TEXTURES];
} C2DRenderer;


internal void
C2DInit(C2DRenderer * Renderer) {
    //TODO(Zen): Must move these files
    Renderer->Shader = CrestShaderInit("../source/C2D/vertex_shader.vs",
                                       "../source/C2D/fragment_shader.fs");

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
    }

    Renderer->BufferIndex = 0;
    Renderer->DrawCalls = 0;
}

internal void
C2DFlush(C2DRenderer * Renderer) {
    glUseProgram(Renderer->Shader);
    glBindTexture(GL_TEXTURE_2D, Renderer->Textures[0]);
    CrestShaderSetFloat(Renderer->Shader, "RendererWidth", Renderer->Width);
    CrestShaderSetFloat(Renderer->Shader, "RendererHeight", Renderer->Height);
    glBindVertexArray(Renderer->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, Renderer->VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, Renderer->BufferIndex * sizeof(C2DVertex), Renderer->Vertices);
    glDrawArrays(GL_TRIANGLES, 0, Renderer->BufferIndex);

    Renderer->BufferIndex = 0;
    Renderer->DrawCalls++;
}

internal void
C2DDrawColouredRect(C2DRenderer * Renderer, v2 Position, v2 Size, v3 Colour) {
    if(Renderer->BufferIndex + 6 > C2D_MAX_VERTICES) C2DFlush(Renderer);

    //Note(Zen): First Triangle
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x, Position.y, 0.0f), Colour, v2(0.0f, 0.0f));
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x + Size.x, Position.y, 0.0f), Colour, v2(1.0f, 0.0f));
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x, Position.y + Size.y, 0.0f), Colour, v2(0.0f, 1.0f));

    //Note(Zen): Second Tangle
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x + Size.x, Position.y + Size.y, 0.0f), Colour, v2(1.0f, 1.0f));
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x + Size.x, Position.y, 0.0f), Colour, v2(1.0f, 0.0f));
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x, Position.y + Size.y, 0.0f), Colour, v2(0.0f, 1.0f));
}


internal void
C2DDrawTexturedRect(C2DRenderer * Renderer, v2 Position, v2 Size) {
    const v3 Colour = v3(1.0f, 1.0f, 1.0f);
    if(Renderer->BufferIndex + 6 > C2D_MAX_VERTICES) C2DFlush(Renderer);

    //Note(Zen): First Triangle
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x, Position.y, 0.0f), Colour, v2(0.0f, 0.0f));
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x + Size.x, Position.y, 0.0f), Colour, v2(1.0f, 0.0f));
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x, Position.y + Size.y, 0.0f), Colour, v2(0.0f, 1.0f));

    //Note(Zen): Second Tangle
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x + Size.x, Position.y + Size.y, 0.0f), Colour, v2(1.0f, 1.0f));
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x + Size.x, Position.y, 0.0f), Colour, v2(1.0f, 0.0f));
    Renderer->Vertices[Renderer->BufferIndex++] = C2DVertex(v3(Position.x, Position.y + Size.y, 0.0f), Colour, v2(0.0f, 1.0f));
}

internal i32
C2DEndFrame(C2DRenderer * Renderer) {
    C2DFlush(Renderer);
    i32 DrawCalls = Renderer->DrawCalls;
    Renderer->DrawCalls = 0;
    return DrawCalls;
}
