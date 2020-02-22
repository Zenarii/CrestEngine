#include <gl/gl.h>
#include "gl/glext.h"
#include "gl/wglext.h"


//OpenGL
//~
global HGLRC GlobalOpenGLContext;
//function pointers
global PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
global PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
global PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

#define OpenGLProc(name, type) PFNGL##type##PROC gl##name;
#include "OpenGLProcedures.inc"


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
Win32OpenGlLoadAllFunctions(void) {
    #define OpenGLProc(name, type) gl##name = (PFNGL##type##PROC)Win32OpenGLLoadFunction("gl" #name);
    #include "OpenGLProcedures.inc"
}


internal b32
Win32OpenGLInit(HDC DeviceContext) {
    b32 success = 0;
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
            success = 1;
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
