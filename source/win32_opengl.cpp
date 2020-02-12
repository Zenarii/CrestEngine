//################
//# Zenarii 2020 #
//################
//TODO move texture generation code into single file
//opengl headers
#include <gl/gl.h>
#include "gl/glext.h"
#include "gl/wglext.h"
#include "win32_opengl.h"
#include "crest_core.h"
#include "CrestMaths.cpp"
#include "graphics/CrestShader.cpp"
#include "graphics/CrestTexture.cpp"


//Temp
#include <stdio.h>
//OpenGL
//~
global_variable HGLRC GlobalOpenGLContext;
//function pointers
global_variable PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
global_variable PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
global_variable PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

internal void *
Win32OpenGLLoadFunction(const char* name) {
    void * p = (void *)wglGetProcAddress(name);
    if (p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1)) {
        //attempt to load OpenGL 1.1 proc
        HMODULE module = LoadLibraryA("opengl32.dll");
        p = GetProcAddress(module, name);
    }
    return p;
}



internal void
Win32OpenGlLoadAllFunctions() {
    #define OpenGLProc(name, type) gl##name = (PFNGL##type##PROC)Win32OpenGLLoadFunction("gl" #name);
    #include "OpenGLProcedures.h"
}



internal bool
Win32OpenGLInit(HDC DeviceContext) {
    bool success = false;
    PIXELFORMATDESCRIPTOR DummyPixelFormatDesc = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1, //version
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, //Flags
        PFD_TYPE_RGBA,
        32, //Color depth
        0, 0, 0, 0, 0, 0,
	    0,
	    0,
	    0,
	    0, 0, 0, 0,
        24,
        8,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };
    int PixelFormat = ChoosePixelFormat(DeviceContext, &DummyPixelFormatDesc);
    SetPixelFormat(DeviceContext, PixelFormat, &DummyPixelFormatDesc);

    HGLRC DummyOpenGLContext = wglCreateContext(DeviceContext);
    wglMakeCurrent(DeviceContext, DummyOpenGLContext);

    //Note(Zen): load the opengl functions
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)Win32OpenGLLoadFunction("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)Win32OpenGLLoadFunction("wglCreateContextAttribsARB");
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)Win32OpenGLLoadFunction("wglSwapIntervalEXT");

    //Note(Zen): create the real pixel format
    {
        //Note(Zen): Attributes taken from: https://www.khronos.org/opengl/wiki/Creating_an_OpenGL_Context_(WGL)#Pixel_Format_Extensions
        int PixelFormatAttributes[] = {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB, 32,
            WGL_DEPTH_BITS_ARB, 24,
            WGL_STENCIL_BITS_ARB, 8,
            0
        };
        UINT AttributeCount = 0;
        wglChoosePixelFormatARB(DeviceContext, PixelFormatAttributes, 0, 1, &PixelFormat, &AttributeCount);
    }

    {
        int ContextAttributes[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            0
        };

        GlobalOpenGLContext = wglCreateContextAttribsARB(DeviceContext, DummyOpenGLContext, ContextAttributes);

        if(GlobalOpenGLContext) {
            wglMakeCurrent(DeviceContext, 0);
            wglDeleteContext(DummyOpenGLContext);
            wglMakeCurrent(DeviceContext, GlobalOpenGLContext);
            wglSwapIntervalEXT(0);
            success = true;
        }

    }
    return success;
}

internal void
Win32OpenGLResize(int width, int height) {
    glViewport(0, 0, width, height);
}

internal void
Win32OpenGLCleanUp(HDC DeviceContext) {
    wglMakeCurrent(DeviceContext, 0);
    wglDeleteContext(GlobalOpenGLContext);
}


//Temp/test code (Won't be platform specific)
//~

global_variable CrestShader shaderProgram;
global_variable unsigned int VAO;
global_variable unsigned int Texture;

internal void InitTriangle() {
    //Note(Zen): Enables transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_BLEND );

    shaderProgram = CrestShaderInit("C:/Dev/Crest/source/VertexShader.vs",
                                    "C:/Dev/Crest/source/FragmentShader.fs");
    float vertices[] = {
        // positions         // texture coords
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f, // top right
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f  // top left
    };

    unsigned int indices[] = {
    0, 1, 3,  // first Triangle
    1, 2, 3   // second Triangle
    };

    unsigned int VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    Texture = CrestTextureInit("../data/PeaceBig.png");

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
}
#include <math.h>

internal void RenderTriangle(real32 Time) {
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, Texture);
    glUseProgram(shaderProgram);
    vector3 offset = vector3(sinf(Time * 0.001f), cosf(Time  * 0.001f), 0.0f);
    CrestShaderSetV3(shaderProgram, "Offset", offset);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
