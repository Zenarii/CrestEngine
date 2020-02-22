#include "renderer.h"

internal vertex
CrestVertexInit(v3 position, v4 colour) {
    vertex result = {position, colour};
    return result;
}
#define vertex(pos, col) CrestVertexInit(pos, col)

internal void
CrestRendererInit(renderer * Renderer) {
    Renderer->shader = CrestShaderInit("../assets/vertexshader.vs", "../assets/fragmentshader.fs");
    glGenVertexArrays(1, &Renderer->VAO);
    glBindVertexArray(Renderer->VAO);

    glGenBuffers(1, &Renderer->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, Renderer->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Renderer->Vertices), Renderer->Vertices, GL_DYNAMIC_DRAW);
    //Note(Zen): Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)0);
    glEnableVertexAttribArray(0);
    //Note(Zen): Colour
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, colour));
    glEnableVertexAttribArray(1);
}

internal void
CrestRendererStartFrame(renderer * Renderer) {
    Renderer->BufferIndex = 0;
}

internal void
CrestPushFilledRect(renderer * Renderer, v4 colour, v2 position, v2 size) {
    //Note(Zen): First Triangle
    Renderer->Vertices[Renderer->BufferIndex++] = vertex(v3(position.x, position.y, 0.0f), colour);
    Renderer->Vertices[Renderer->BufferIndex++] = vertex(v3(position.x + size.x, position.y, 0.0f), colour);
    Renderer->Vertices[Renderer->BufferIndex++] = vertex(v3(position.x, position.y + size.y, 0.0f), colour);

    //Note(Zen): Second Triangle
    Renderer->Vertices[Renderer->BufferIndex++] = vertex(v3(position.x + size.x, position.y + size.y, 0.0f), colour);
    Renderer->Vertices[Renderer->BufferIndex++] = vertex(v3(position.x + size.x, position.y, 0.0f), colour);
    Renderer->Vertices[Renderer->BufferIndex++] = vertex(v3(position.x, position.y + size.y, 0.0f), colour);
}

internal void
CrestPushLine(renderer * Renderer, v4 colour, v2 start, v2 end) {
    Renderer->Vertices[Renderer->BufferIndex++] = vertex(v3(start.x - LINE_THICKNESS, start.y + LINE_THICKNESS, 0.0f), colour);
    Renderer->Vertices[Renderer->BufferIndex++] = vertex(v3(end.x + LINE_THICKNESS, end.y + LINE_THICKNESS, 0.0f), colour);
    Renderer->Vertices[Renderer->BufferIndex++] = vertex(v3(start.x - LINE_THICKNESS, start.y - LINE_THICKNESS, 0.0f), colour);

    Renderer->Vertices[Renderer->BufferIndex++] = vertex(v3(start.x + LINE_THICKNESS, start.y + LINE_THICKNESS, 0.0f), colour);
    Renderer->Vertices[Renderer->BufferIndex++] = vertex(v3(end.x + LINE_THICKNESS, end.y - LINE_THICKNESS, 0.0f), colour);
    Renderer->Vertices[Renderer->BufferIndex++] = vertex(v3(end.x + LINE_THICKNESS, end.y + LINE_THICKNESS, 0.0f), colour);
}

internal void
CrestPushBorder(renderer * Renderer, v4 colour, v2 position, v2 size) {
    CrestPushLine(Renderer, colour, position, v2(position.x+size.x, position.y));
    CrestPushLine(Renderer, colour, v2(position.x+size.x, position.y), v2(position.x+size.x, position.y+size.y));
    CrestPushLine(Renderer, colour, v2(position.x+size.x, position.y+size.y), v2(position.x, position.y+size.y));
    CrestPushLine(Renderer, colour, v2(position.x, position.y+size.y), position);
}

internal void
CrestRender(renderer * Renderer) {
    glUseProgram(Renderer->shader);
    CrestShaderSetFloat(Renderer->shader, "RendererWidth", Renderer->Width);
    CrestShaderSetFloat(Renderer->shader, "RendererHeight", Renderer->Height);
    glBindVertexArray(Renderer->VAO);

    glBufferSubData(GL_ARRAY_BUFFER, 0, Renderer->BufferIndex*sizeof(vertex), Renderer->Vertices);
    glDrawArrays(GL_TRIANGLES, 0, Renderer->BufferIndex);

    glBindVertexArray(0);
}
