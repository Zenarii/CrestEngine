typedef struct fbo fbo;
struct fbo {
    u32 Fbo;
    u32 Texture;
    b32 UseDepthTexture;
    union {
        u32 Rbo;
        u32 DepthTexture;
    };
    i32 Width;
    i32 Height;
};

internal fbo
CrestCreateFramebuffer(i32 Width, i32 Height, b32 UseDepthTexture) {
    u32 FBO = 0;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    //Bind Colour Attachment texture
    u32 FrameBufferTexture = 0;
    glGenTextures(1, &FrameBufferTexture);
    glBindTexture(GL_TEXTURE_2D, FrameBufferTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FrameBufferTexture, 0);

    u32 DepthComponent = 0;
    if(UseDepthTexture) {
        glGenTextures(1, &DepthComponent);
        glBindTexture(GL_TEXTURE_2D, DepthComponent);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, Width, Height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, DepthComponent, 0);
        Assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE && "With Depth Texture");
    }
    else { //Use Renderbuffer
        glGenRenderbuffers(1, &DepthComponent);
        glBindRenderbuffer(GL_RENDERBUFFER, DepthComponent);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, Width, Height);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, DepthComponent);
        Assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE && "With Renderbuffer");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    fbo Result = {
        .Fbo = FBO,
        .Texture = FrameBufferTexture,
        .UseDepthTexture = UseDepthTexture,
        .Rbo = DepthComponent,
        .Width = Width,
        .Height = Width
    };
    return Result;
}

internal void
CrestDeleteFramebuffer(fbo Fbo) {
    glDeleteFramebuffers(1, &Fbo.Fbo);
    glDeleteTextures(1, &Fbo.Texture);
    glDeleteRenderbuffers(1, &Fbo.Rbo);
}
