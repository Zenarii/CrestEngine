#include "ui_renderer.h"



internal vertex
CrestVertexInit(v3 position, v4 colour) {
    vertex result = {position, colour};
    return result;
}

internal textured_vertex
CrestTVertexInit(v3 position, v2 texcoord) {
    textured_vertex result = {position, texcoord};
    return result;
}

internal void
CrestUIRendererInit(ui_renderer * UIRenderer) {
    UIRenderer->shader = CrestShaderInit("../assets/ui_vertexshader.vs", "../assets/ui_fragmentshader.fs");
    UIRenderer->TextShader = CrestShaderInit("../assets/ui_text_vertexshader.vs", "../assets/ui_text_fragmentshader.fs");
    //Note(Zen): Setup the shape drawing stuff

    {
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

    //Note(Zen): Setup the text drawing
    {
        glGenVertexArrays(1, &UIRenderer->TVAO);
        glBindVertexArray(UIRenderer->TVAO);

        glGenBuffers(1, &UIRenderer->TVBO);
        glBindBuffer(GL_ARRAY_BUFFER, UIRenderer->TVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(UIRenderer->TextVertices), UIRenderer->TextVertices, GL_DYNAMIC_DRAW);
        //Note(Zen): Position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(textured_vertex), (void*)0);
        glEnableVertexAttribArray(0);
        //Note(Zen): Colour
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(textured_vertex), (void*)offsetof(textured_vertex, texcoord));
        glEnableVertexAttribArray(1);
    }

    glBindVertexArray(0);
}

internal void
CrestUIRendererStartFrame(ui_renderer * UIRenderer) {
    UIRenderer->BufferIndex = 0;
    UIRenderer->TextBufferIndex = 0;
    UIRenderer->TransparentBufferIndex = 0;
}


internal void
CrestUIRendererLoadFont(ui_renderer * UIRenderer, const char * FontPath) {
    char * TTFBuffer = CrestLoadFileAsString(FontPath);
    unsigned char TempBitmap[512*512];
    stbtt_BakeFontBitmap(TTFBuffer, 0, 20.0, TempBitmap, 512, 512, 32, 96, UIRenderer->CharacterData);


    glGenTextures(1, &UIRenderer->FontTex);
    glBindTexture(GL_TEXTURE_2D, UIRenderer->FontTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512,512, 0, GL_RED, GL_UNSIGNED_BYTE, TempBitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);



    free(TTFBuffer);
}

internal void
CrestPushText(ui_renderer * UIRenderer, v3 Position, const char * Text) {
    //glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, UIRenderer->FontTex);

    float x = Position.x;
    float y = Position.y;

    while(*Text) {
        if((*Text >= 32) && (*Text < 128)) {
            stbtt_aligned_quad Quad;
            stbtt_GetBakedQuad(UIRenderer->CharacterData, 512, 512, *Text-32, &x, &y, &Quad, 1);

            //TODO(Zen): Pull this out into it's own function?
            UIRenderer->TextVertices[UIRenderer->TextBufferIndex++] = textured_vertex(v3(Quad.x0, Quad.y0, Position.z), v2(Quad.s0, Quad.t0));
            UIRenderer->TextVertices[UIRenderer->TextBufferIndex++] = textured_vertex(v3(Quad.x1, Quad.y0, Position.z), v2(Quad.s1, Quad.t0));
            UIRenderer->TextVertices[UIRenderer->TextBufferIndex++] = textured_vertex(v3(Quad.x0, Quad.y1, Position.z), v2(Quad.s0, Quad.t1));

            UIRenderer->TextVertices[UIRenderer->TextBufferIndex++] = textured_vertex(v3(Quad.x1, Quad.y0, Position.z), v2(Quad.s1, Quad.t0));
            UIRenderer->TextVertices[UIRenderer->TextBufferIndex++] = textured_vertex(v3(Quad.x1, Quad.y1, Position.z), v2(Quad.s1, Quad.t1));
            UIRenderer->TextVertices[UIRenderer->TextBufferIndex++] = textured_vertex(v3(Quad.x0, Quad.y1, Position.z), v2(Quad.s0, Quad.t1));

        }
        ++Text;
    }
    Assert(UIRenderer->TextBufferIndex < MAX_TEXT_VERTICES_SIZE);
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
    Assert(UIRenderer->BufferIndex < MAX_VERTICES_SIZE);
}

internal void
CrestPushFilledRectD(ui_renderer * UIRenderer, v4 colour, v3 position, v2 size) {
    //Note(Zen): First Triangle
    UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x, position.y, position.z), colour);
    UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x + size.x, position.y, position.z), colour);
    UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x, position.y + size.y, position.z), colour);

    //Note(Zen): Second Triangle
    UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x + size.x, position.y + size.y, position.z), colour);
    UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x + size.x, position.y, position.z), colour);
    UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x, position.y + size.y, position.z), colour);
    Assert(UIRenderer->BufferIndex < MAX_VERTICES_SIZE);
}

//Note(Zen): Makes sure transparent rectangles are rendered last
internal void
CrestPushTransparentRect(ui_renderer * UIRenderer, v4 colour, v3 position, v2 size) {
    //Note(Zen): First Triangle
    UIRenderer->TransparentVertices[UIRenderer->TransparentBufferIndex++] = vertex(v3(position.x, position.y, position.z), colour);
    UIRenderer->TransparentVertices[UIRenderer->TransparentBufferIndex++] = vertex(v3(position.x + size.x, position.y, position.z), colour);
    UIRenderer->TransparentVertices[UIRenderer->TransparentBufferIndex++] = vertex(v3(position.x, position.y + size.y, position.z), colour);

    //Note(Zen): Second Triangle
    UIRenderer->TransparentVertices[UIRenderer->TransparentBufferIndex++] = vertex(v3(position.x + size.x, position.y + size.y, position.z), colour);
    UIRenderer->TransparentVertices[UIRenderer->TransparentBufferIndex++] = vertex(v3(position.x + size.x, position.y, position.z), colour);
    UIRenderer->TransparentVertices[UIRenderer->TransparentBufferIndex++] = vertex(v3(position.x, position.y + size.y, position.z), colour);
    Assert(UIRenderer->TransparentBufferIndex < MAX_VERTICES_SIZE);
}

internal void
CrestPushBorder(ui_renderer * UIRenderer, v4 colour, v3 position, v2 size) {
    //Note(Zen): Top Border
    {
        UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x - 0.5f,          position.y - 1.5f, position.z), colour);
        UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x - 0.5f,          position.y - 0.5f, position.z), colour);
        UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x + size.x + 1.5f, position.y - 1.5f, position.z), colour);

        UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x - 0.5f,          position.y - 0.5f, position.z), colour);
        UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x + size.x + 1.5f, position.y - 1.5f, position.z), colour);
        UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x + size.x + 1.5f, position.y - 0.5f, position.z), colour);
    }
    //Note(Zen): Bottom Border
    {
        UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x - 0.5f,          position.y + size.y - 0.5f, position.z), colour);
        UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x - 0.5f,          position.y + size.y + 0.5f, position.z), colour);
        UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x + size.x + 0.5f, position.y + size.y - 0.5f, position.z), colour);

        UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x - 0.5f,          position.y + size.y + 0.5f, position.z), colour);
        UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x + size.x + 0.5f, position.y + size.y - 0.5f, position.z), colour);
        UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x + size.x + 0.5f, position.y + size.y + 0.5f, position.z), colour);
    }
    //Note(Zen): Left Border
    {
        UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x - 0.5f, position.y - 0.5f, position.z), colour);
        UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x + 0.5f, position.y - 0.5f, position.z), colour);
        UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x - 0.5f, position.y + size.y + 0.5f, position.z), colour);


        UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x + 0.5f, position.y - 0.5f, position.z), colour);
        UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x - 0.5f, position.y + size.y + 0.5f, position.z), colour);
        UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x + 0.5f, position.y + size.y + 0.5f, position.z), colour);
    }
    //Note(Zen): Right Border
    {
        UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x + size.x + 0.5f, position.y - 0.5f, position.z), colour);
        UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x + size.x + 1.5f, position.y - 0.5f, position.z), colour);
        UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x + size.x + 0.5f, position.y + size.y + 0.5f, position.z), colour);


        UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x + size.x + 1.5f, position.y - 0.5f, position.z), colour);
        UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x + size.x + 0.5f, position.y + size.y + 0.5f, position.z), colour);
        UIRenderer->Vertices[UIRenderer->BufferIndex++] = vertex(v3(position.x + size.x + 1.5f, position.y + size.y + 0.5f, position.z), colour);
    }

    Assert(UIRenderer->BufferIndex < MAX_VERTICES_SIZE);
}

internal void
CrestUIRender(ui_renderer * UIRenderer) {
    //Note(Zen): Draw Rectangles
    {
        glUseProgram(UIRenderer->shader);
        CrestShaderSetFloat(UIRenderer->shader, "RendererWidth", UIRenderer->Width);
        CrestShaderSetFloat(UIRenderer->shader, "RendererHeight", UIRenderer->Height);
        glBindVertexArray(UIRenderer->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, UIRenderer->VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, UIRenderer->BufferIndex * sizeof(vertex), UIRenderer->Vertices);
        glBufferSubData(GL_ARRAY_BUFFER, UIRenderer->BufferIndex * sizeof(vertex), UIRenderer->TransparentBufferIndex*sizeof(vertex), UIRenderer->TransparentVertices);
        glDrawArrays(GL_TRIANGLES, 0, UIRenderer->BufferIndex + UIRenderer->TransparentBufferIndex);
    }

    //Note(Zen): Draw text
    {
        glUseProgram(UIRenderer->TextShader);
        //glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, UIRenderer->FontTex);
        CrestShaderSetFloat(UIRenderer->TextShader, "RendererWidth", UIRenderer->Width);
        CrestShaderSetFloat(UIRenderer->TextShader, "RendererHeight", UIRenderer->Height);
        glBindVertexArray(UIRenderer->TVAO);
        glBindBuffer(GL_ARRAY_BUFFER, UIRenderer->TVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, UIRenderer->TextBufferIndex*sizeof(textured_vertex), UIRenderer->TextVertices);
        glDrawArrays(GL_TRIANGLES, 0, UIRenderer->TextBufferIndex);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
