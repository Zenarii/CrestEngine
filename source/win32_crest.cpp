//Zenarii 2020

#include <windows.h>
#include "win32_opengl.cpp"
#include "crest_core.h"
#include "platform.cpp"
//own headers
#include "win32_crest.h"
#include "crest.cpp"

global_variable bool running;
global_variable bool OpenGLHasLoaded;

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
        case WM_ACTIVATEAPP: {} break;
        case WM_SIZE: {
            win32_window_dimension WindowDimension = Win32GetWindowDimension(windowHandle);
            Win32OpenGLResize(WindowDimension.Width, WindowDimension.Height);
            /*
            if(OpenGLHasLoaded) {
                win32_window_dimension WindowDimension = Win32GetWindowDimension(windowHandle);
                Win32OpenGLResize(WindowDimension.Width, WindowDimension.Height);
                HDC DeviceContext = GetDC(windowHandle);
                GameUpdateAndRender();
                SwapBuffers(DeviceContext);
            }*/
        } break;
        case WM_SYSKEYDOWN: {} break;
        case WM_SYSKEYUP: {} break;
        case WM_KEYDOWN: {} break;
        case WM_KEYUP: {} break;
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
    WNDCLASSA windowClass = {};
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = Win32MainWindowCallback;
    windowClass.hInstance = instance;
    //windowClass.hIcon;
    windowClass.lpszClassName = "CrestWindowClass";
    platform Platform = {};
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

        if(windowHandle) {
            LARGE_INTEGER ClockFrequencyResult;
            QueryPerformanceFrequency(&ClockFrequencyResult);
            int64 ClockFrequency = ClockFrequencyResult.QuadPart;

            HDC deviceContext = GetDC(windowHandle);
            OpenGLHasLoaded = Win32OpenGLInit(deviceContext);
            Win32OpenGlLoadAllFunctions();
            //TEMP
            InitTriangle();

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

                GameUpdateAndRender(&Platform);
                SwapBuffers(deviceContext);

                int64 EndCycleCount;
                EndCycleCount = __rdtsc();

                LARGE_INTEGER EndCounter;
                QueryPerformanceCounter(&EndCounter);
                if(running) {
                    int64 CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
                    real32 msPerFrame = ((1000*CounterElapsed) / ClockFrequency);
                    int32 imsPerFrame = (int32) ((1000*CounterElapsed) / ClockFrequency);
                    int32 FPS = ClockFrequency/CounterElapsed;
                    char windowTitle[32];
                    wsprintf(windowTitle, "Crest ms/frame: %d | FPS: %d", imsPerFrame, FPS);
                    SetWindowTextA(windowHandle, windowTitle);

                    Platform.Delta = msPerFrame;
                    Platform.TotalTime += msPerFrame;
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
