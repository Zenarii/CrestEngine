
internal u32
CasLoadTexture(const char * Path, GLenum Filter) {
    u32 Texture;
    glGenTextures(1, &Texture);
    glBindTexture(GL_TEXTURE_2D, Texture);

    //Note(Zen): Set the wrapping options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //Note(Zen): Set Filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filter);

    i32 Width, Height, NumberOfChannels;
    unsigned char * Data = stbi_load(Path, &Width, &Height, &NumberOfChannels, 0);




    if(Data) {
        if (NumberOfChannels == 4) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else if (NumberOfChannels == 3) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,  Width, Height, 0, GL_RGB,  GL_UNSIGNED_BYTE, Data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else if(NumberOfChannels == 2) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG,  Width, Height, 0, GL_RG,  GL_UNSIGNED_BYTE, Data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else if(NumberOfChannels == 1) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,  Width, Height, 0, GL_RED,  GL_UNSIGNED_BYTE, Data);
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
