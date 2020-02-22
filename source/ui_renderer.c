#include "ui_renderer.h"

internal vertex
CrestVertexInit(v3 position, v4 colour) {
    vertex result = {position, colour};
    return result;
}
#define vertex(pos, col) CrestVertexInit(pos, col)

internal void
CrestUIRendererInit(ui_renderer * UIRenderer) {
    UIRenderer->shader = CrestShaderInit("../assets/vertexshader.vs", "../assets/fragmentshader.fs");
    glGenVertexArrays(1, &UIRenderer->VAO);
    glBindVertexArray(UIRenderer->VAO);

    glGenBuffers(1, &UIRenderer->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, UIRenderer->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(UIRenderer->Vertices), UIRenderer->Vertices, GL_DYNAMIC_DRAW);
    //Note(Zen): Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)0);
    glEnableVertexAttribArray(0);
    //Note(Zen): Colour
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, colour));
    glEnableVertexAttribArray(1);
}

internal void
CrestUIRendererStartFrame(ui_renderer * UIRenderer) {
    UIRenderer->BufferIndex = 0;
}

internal void
CrestPushFilledRect(ui_renderer * UIRenderer, v4 colour, v2 position, v2 size) {
    //Note(Zen): First Triangle
    UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x, position.y, 0.0f), colour);
    UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x + size.x, position.y, 0.0f), colour);
    UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x, position.y + size.y, 0.0f), colour);

    //Note(Zen): Second Triangle
    UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x + size.x, position.y + size.y, 0.0f), colour);
    UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x + size.x, position.y, 0.0f), colour);
    UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x, position.y + size.y, 0.0f), colour);
}

internal void
CrestRender(ui_renderer * UIRenderer) {
    glUseProgram(UIRenderer->shader);
    CrestShaderSetFloat(UIRenderer->shader, "RendererWidth", UIRenderer->Width);
    CrestShaderSetFloat(UIRenderer->shader, "RendererHeight", UIRenderer->Height);
    glBindVertexArray(UIRenderer->VAO);

    glBufferSubData(GL_ARRAY_BUFFER, 0, UIRenderer->BufferIndex*sizeof(vertex), UIRenderer->Vertices);
    glDrawArrays(GL_TRIANGLES, 0, UIRenderer->BufferIndex);

    glBindVertexArray(0);
}
