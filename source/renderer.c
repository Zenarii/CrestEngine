




#define MAX_RECTS 256
//Note(Zen): max rects * (six vertices per rect)
#define MAX_VERTICES_SIZE MAX_RECTS * 6

typedef struct vertex {
    v3 position;
    v4 colour;
} vertex;

internal vertex
CrestVertexInit(v3 position, v4 colour) {
    vertex result = {position, colour};
    return result;
}
#define vertex(pos, col) CrestVertexInit(pos, col)

global i32 BufferIndex;
global vertex Vertices[MAX_VERTICES_SIZE];
global u32 VAO, VBO;
global CrestShader shader;

internal void
CrestRendererInit(void) {
    shader = CrestShaderInit("../assets/vertexshader.vs", "../assets/fragmentshader.fs");
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_DYNAMIC_DRAW);
    //Note(Zen): Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)0);
    glEnableVertexAttribArray(0);
    //Note(Zen): Colour
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, colour));
    glEnableVertexAttribArray(1);
}

internal void
CrestRendererStartFrame(void) {
    BufferIndex = 0;
}

internal void
CrestPushFilledRect(v4 colour, v2 position, v2 size) {
    //Note(Zen): First Triangle
    Vertices[BufferIndex++] = vertex(v3(position.x, position.y, 0.0f), colour);
    Vertices[BufferIndex++] = vertex(v3(position.x + size.x, position.y, 0.0f), colour);
    Vertices[BufferIndex++] = vertex(v3(position.x, position.y + size.y, 0.0f), colour);

    //Note(Zen): Second Triangle
    Vertices[BufferIndex++] = vertex(v3(position.x + size.x, position.y + size.y, 0.0f), colour);
    Vertices[BufferIndex++] = vertex(v3(position.x + size.x, position.y, 0.0f), colour);
    Vertices[BufferIndex++] = vertex(v3(position.x, position.y + size.y, 0.0f), colour);
}

internal void
CrestRender(void) {
    glUseProgram(shader);
    glBindVertexArray(VAO);

    glBufferSubData(GL_ARRAY_BUFFER, 0, BufferIndex*sizeof(vertex), Vertices);
    glDrawArrays(GL_TRIANGLES, 0, BufferIndex);

    glBindVertexArray(0);
}
