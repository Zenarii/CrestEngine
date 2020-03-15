internal u32
CasLoadWhiteTexture() {
    //Note(Zen): Set the wrapping options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    const v4 BorderColour = {1.f, 1.f, 0.f, 1.f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &BorderColour.elements[0]);

    //Note(Zen): Set Filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    u8 Data[3] = {255, 255, 255};

    u32 Texture;
    glGenTextures(1, &Texture);
    glBindTexture(GL_TEXTURE_2D, Texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, Data);
    glGenerateMipmap(GL_TEXTURE_2D);

    return Texture;
}

internal u32
CasLoadTexture(const char * Path) {
    //Note(Zen): Set the wrapping options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    const v4 BorderColour = {1.f, 1.f, 0.f, 1.f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &BorderColour.elements[0]);

    //Note(Zen): Set Filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    i32 Width, Height, NumberOfChannels;
    unsigned char * Data = stbi_load(Path, &Width, &Height, &NumberOfChannels, 0);

    u32 Texture;
    glGenTextures(1, &Texture);

    glBindTexture(GL_TEXTURE_2D, Texture);
    if(Data) {
        if (NumberOfChannels == 4) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else if (NumberOfChannels == 3) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,  Width, Height, 0, GL_RGB,  GL_UNSIGNED_BYTE, Data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            Assert(!"Incorrect number of channels in image path");
        }
    }
    else {
        //TODO(Zen): Logging
    }

    stbi_image_free(Data);
    return Texture;
}

internal u32
CasLoadFont() {

}
