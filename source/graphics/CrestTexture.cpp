#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

typedef unsigned int CrestTexture;

internal CrestTexture
CrestTextureInit(const char * Path) {
    CrestTexture Result;
    glGenTextures(1, &Result);
    glBindTexture(GL_TEXTURE_2D, Result);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int Width, Height, Channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char * data = stbi_load(Path, &Width, &Height, &Channels, 0);
    if(data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
    else {
        CrestLog("Failed to load image from: ");
        CrestLog(Path);
        CrestLog("\n");
    }
    stbi_image_free(data);
    return Result;
}
