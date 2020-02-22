#ifndef UI_RENDERER_H
#define UI_RENDERER_H

#define MAX_RECTS 256
//Note(Zen): max rects * (six vertices per rect)
#define MAX_VERTICES_SIZE MAX_RECTS * 6
#define LINE_THICKNESS 1.0f

typedef struct vertex {
    v3 position;
    v4 colour;
} vertex;

internal vertex
CrestVertexInit(v3 position, v4 colour);
#define vertex(pos, col) CrestVertexInit(pos, col)


typedef struct ui_renderer {
    r32 Width, Height;
    i32 BufferIndex;
    vertex Vertices[MAX_VERTICES_SIZE];
    u32 VAO, VBO;
    CrestShader shader;
} ui_renderer;

#endif
