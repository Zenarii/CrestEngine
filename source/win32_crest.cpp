//Zenarii 2020

#include <windows.h>
#include <gl/gl.h>
#include "gl/wglext.h"
#include "crest_core.h"
#include "win32_crest.h"
#include "crest.cpp"

global_variable bool running;
global_variable win32_offscreen_buffer GlobalBackBuffer;


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
Win32OpenGLCleanUp(HDC DeviceContext) {
    wglMakeCurrent(DeviceContext, 0);
    wglDeleteContext(GlobalOpenGLContext);
}

//Window Buffer
//~
internal win32_window_dimension
Win32GetWindowDimension(HWND windowHandle) {
    win32_window_dimension Result;

    RECT clientRect;
    GetClientRect(windowHandle, &clientRect);
    Result.Width = clientRect.right - clientRect.left;
    Result.Height = clientRect.bottom - clientRect.top;
    return Result;
}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer *buffer, int width, int height) {
    if(buffer->Memory) {
        VirtualFree(buffer->Memory, 0, MEM_RELEASE);
    }

    buffer->Width = width;
    buffer->Height = height;
    buffer->bytesPerPixel = 4;

    buffer->Info.bmiHeader.biSize = sizeof(buffer->Info.bmiHeader);
    buffer->Info.bmiHeader.biWidth = buffer->Width;
    buffer->Info.bmiHeader.biHeight = -buffer->Height;
    buffer->Info.bmiHeader.biPlanes = 1;
    buffer->Info.bmiHeader.biBitCount = 32;
    buffer->Info.bmiHeader.biCompression = BI_RGB;

    int bitmapMemorySize = buffer->bytesPerPixel * (buffer->Width * buffer->Height);
    buffer->Memory = VirtualAlloc(0, bitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    buffer->Pitch = buffer->Width * buffer->bytesPerPixel;
}

internal void
Win32DisplayBufferToWindow(HDC deviceContext, int WindowWidth, int WindowHeight,
                           win32_offscreen_buffer * buffer) {
    StretchDIBits(deviceContext,
                  0, 0, WindowWidth, WindowHeight,
                  0, 0, buffer->Width, buffer->Height,
                  buffer->Memory,
                  &buffer->Info,
                  DIB_RGB_COLORS, SRCCOPY);


}

//Windows Callbacks
//~

LRESULT CALLBACK Win32MainWindowCallback(
    HWND windowHandle,
    UINT message,
    WPARAM WParam,
    LPARAM LParam)
{
    LRESULT result = 0;

    switch(message) {
        case WM_DESTROY: {
            running = false;
        } break;
        case WM_CLOSE: {
            running = false;
        } break;
        case WM_QUIT: {
            running = false;
        } break;
        case WM_ACTIVATEAPP: {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;
        case WM_PAINT: {
            PAINTSTRUCT Paint;
            HDC deviceContext = BeginPaint(windowHandle, &Paint);
            int x = Paint.rcPaint.left;
            int y = Paint.rcPaint.bottom;
            int width = Paint.rcPaint.right - Paint.rcPaint.left;
            int height = Paint.rcPaint.bottom - Paint.rcPaint.top;

            win32_window_dimension WindowDimension = Win32GetWindowDimension(windowHandle);

            Win32DisplayBufferToWindow(deviceContext, WindowDimension.Width,
                WindowDimension.Height, &GlobalBackBuffer);

            EndPaint(windowHandle, &Paint);
            ValidateRect(windowHandle, NULL);
        } break;
        case WM_SYSKEYDOWN: {} break;
        case WM_SYSKEYUP: {
        } break;
        case WM_KEYDOWN: {} break;
        case WM_KEYUP: {
            uint32 VirtualKeyCode = WParam;
            //30th bit of lParam shows whether down before
            bool wasDown = ((LParam & (1<<30)) != 0);
            bool isDown = ((LParam & (1<<31)) == 0);

            if (VirtualKeyCode == 'A') {
                OutputDebugStringA("A");
            }
            else if (VirtualKeyCode == 'W') {

            }

        } break;
        default: {
            result = DefWindowProc(windowHandle, message, WParam, LParam);
        } break;
    }
    return result;
}

int CALLBACK WinMain(
    HINSTANCE instance,
    HINSTANCE previousInstance,
    LPSTR commandLine,
    int cmdShow)
{
    Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

    WNDCLASSA windowClass = {};
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = Win32MainWindowCallback;
    windowClass.hInstance = instance;
    //windowClass.hIcon;
    windowClass.lpszClassName = "CrestWindowClass";

    //Create the window
    if(RegisterClass(&windowClass)) {
        HWND windowHandle = CreateWindowEx(
                0,
                windowClass.lpszClassName,
                "CREST",
                WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                0,
                0,
                instance,
                0);
        int xOffset = 0;
        int yOffset = 30;


        if(windowHandle) {
            LARGE_INTEGER ClockFrequencyResult;
            QueryPerformanceFrequency(&ClockFrequencyResult);
            int64 ClockFrequency = ClockFrequencyResult.QuadPart;

            HDC deviceContext = GetDC(windowHandle);
            Win32OpenGLInit(deviceContext);

            LARGE_INTEGER LastCounter;
            QueryPerformanceCounter(&LastCounter);
            int64 LastCycleCount;
            LastCycleCount = __rdtsc();

            running = true;
            while(running){
                //Process Windows Messages
                MSG message;
                while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
                    if(message.message == WM_QUIT) running = false;

                    BOOL messageResult = GetMessage(&message, 0, 0, 0);

                    TranslateMessage(&message);
                    DispatchMessageA(&message);
                }

                win32_window_dimension WindowDimension = Win32GetWindowDimension(windowHandle);

                crest_offscreen_buffer CrestScreenBuffer = {};
                CrestScreenBuffer.Memory = GlobalBackBuffer.Memory;
                CrestScreenBuffer.Width = GlobalBackBuffer.Width;
                CrestScreenBuffer.Height = GlobalBackBuffer.Height;
                CrestScreenBuffer.Pitch = GlobalBackBuffer.Pitch;
                CrestScreenBuffer.BytesPerPixel = GlobalBackBuffer.bytesPerPixel;

                GameUpdateAndRender(&CrestScreenBuffer);
                Win32DisplayBufferToWindow(deviceContext, WindowDimension.Width,
                    WindowDimension.Height, &GlobalBackBuffer);

                int64 EndCycleCount;
                EndCycleCount = __rdtsc();

                LARGE_INTEGER EndCounter;
                QueryPerformanceCounter(&EndCounter);
                if(running) {
                    int64 CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
                    int32 msPerFrame = (int32)((1000*CounterElapsed) / ClockFrequency);
                    int32 FPS = ClockFrequency/CounterElapsed;
                    char windowTitle[32];
                    wsprintf(windowTitle, "Crest ms/frame: %d | FPS: %d", msPerFrame, FPS);
                    SetWindowTextA(windowHandle, windowTitle);
                }
                LastCounter = EndCounter;
            }
        } else {
            //TODO log failure
        }
    } else {
        //TODO log failure
    }
    return 0;
}
//~
