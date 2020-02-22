#define MAX_RECTS 256
//Note(Zen): max rects * (six vertices per rect)
#define MAX_VERTICES_SIZE MAX_RECTS * 6
#define LINE_THICKNESS 1.0f

typedef struct vertex {
    v3 position;
    v4 colour;
} vertex;

typedef struct renderer {
    r32 Width, Height;
    i32 BufferIndex;
    vertex Vertices[MAX_VERTICES_SIZE];
    u32 VAO, VBO;
    CrestShader shader;
} renderer;
