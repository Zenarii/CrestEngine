#ifndef UI_RENDERER_H
#define UI_RENDERER_H

#define MAX_RECTS 256
//Note(Zen): max rects * (six vertices per rect)
#define MAX_VERTICES_SIZE MAX_RECTS * 6

#define BUTTON_COLOUR v4(75.f/255.f, 75.f/255.f, 75.f/255.f, 1.f)
#define BUTTON_HOVER_COLOUR v4(95.f/255.f, 95.f/255.f, 95.f/255.f,  1.f)

typedef struct vertex {
    v3 position;
    v4 colour;
} vertex;

internal vertex
CrestVertexInit(v3 position, v4 colour);
#define vertex(pos, col) CrestVertexInit(pos, col)

typedef struct textured_vertex {
    v3 position;
    v2 texcoord;
} textured_vertex;

internal textured_vertex
CrestTVertexInit(v3 position, v2 texcoord);
#define textured_vertex(pos, tex) CrestTVertexInit(pos, tex)

typedef struct ui_renderer {
    //Note(Zen): For shapes etc.
    r32 Width, Height;
    i32 BufferIndex;
    vertex Vertices[MAX_VERTICES_SIZE];
    u32 VAO, VBO;
    CrestShader shader;

    //Note(Zen): For text
    stbtt_bakedchar CharacterData[96];
    u32 FontTex;
    i32 TextBufferIndex;
    textured_vertex TextVertices[MAX_VERTICES_SIZE];
    u32 TVAO, TVBO;
    CrestShader TextShader;
} ui_renderer;

#endif
