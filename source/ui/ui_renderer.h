/*
TODO(Zen): May be able to compress the text into the same vertices buffer as the
filled rectangles, reducing the number of draw calls
*/

#ifndef UI_RENDERER_H
#define UI_RENDERER_H

#define MAX_RECTS 256
//Note(Zen): max rects * (six vertices per rect)
#define MAX_VERTICES_SIZE MAX_RECTS * 6
#define MAX_TEXT_VERTICES_SIZE MAX_VERTICES_SIZE * 4

#define UI_FONT_HEIGHT 20.f

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
    i32 TransparentBufferIndex;
    vertex TransparentVertices[MAX_VERTICES_SIZE];

    //Note(Zen): For text
    stbtt_bakedchar CharacterData[96];
    stbtt_fontinfo Font;
    v4 FontRect;
    r32 FontScale;
    u32 FontTex;
    i32 TextBufferIndex;
    textured_vertex TextVertices[MAX_TEXT_VERTICES_SIZE];
    u32 TVAO, TVBO;
    CrestShader TextShader;
} ui_renderer;

#endif
