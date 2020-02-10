//################
//# Zenarii 2020 #
//################

//opengl headers
#include <gl/gl.h>
#include "gl/glext.h"
#include "gl/wglext.h"
#include "crest_core.h"

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

#define OpenGLProc(name, type) PFNGL##type##PROC gl##name;
#include "OpenGLProcedures.h"

internal void
Win32OpenGlLoadAllFunctions() {
    #define OpenGLProc(name, type) gl##name = (PFNGL##type##PROC)Win32OpenGLLoadFunction("gl" #name);
    #include "OpenGLProcedures.h"
}

internal void
Win32OpenGLInit(HDC DeviceContext) {
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
        }

    }
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


//Temp/test code
//~
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}";


internal void RenderTriangle() {
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };

    unsigned int VBO;
    glGenBuffers(1, &VBO);

    unsigned int VertexShader = 0;
    VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShader, 1, &vertexShaderSource, 0);
    glCompileShader(VertexShader);

    int success;
    char infoLog[512];

    glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(VertexShader, 512, NULL, infoLog);
        OutputDebugStringA(infoLog);
    }

    unsigned int FragmentShader = 0;
    FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShader, 1, &fragmentShaderSource, 0);
    glCompileShader(FragmentShader);

    glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(FragmentShader, 512, NULL, infoLog);
        OutputDebugStringA(infoLog);
    }

    unsigned int ShaderProgram = 0;
    ShaderProgram = glCreateProgram();
    glAttachShader(ShaderProgram, VertexShader);
    glAttachShader(ShaderProgram, FragmentShader);
    glLinkProgram(ShaderProgram);

    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(ShaderProgram, 512, NULL, infoLog);
        OutputDebugStringA(infoLog);
    }

    glUseProgram(ShaderProgram);
    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glUseProgram(ShaderProgram);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
