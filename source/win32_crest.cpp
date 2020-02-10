//Zenarii 2020

#include <windows.h>
#include "win32_opengl.cpp"
//own headers
#include "crest_core.h"
#include "win32_crest.h"
#include "crest.cpp"

global_variable bool running;
global_variable win32_offscreen_buffer GlobalBackBuffer;


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
            SwapBuffers(deviceContext);

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
            Win32OpenGlLoadAllFunctions();

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
                GameUpdateAndRender();
                SwapBuffers(deviceContext);
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
