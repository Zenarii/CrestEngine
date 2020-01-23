//Zenarii 2020
#include <windows.h>
#include <stdint.h>
#include <xinput.h>

#define local_persist static
#define global_variable static
#define internal static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

//XINPUT stubs
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
X_INPUT_GET_STATE(XInputGetStateStub) {
    return 0;
}
typedef X_INPUT_GET_STATE(x_input_get_state);
global_variable x_input_get_state* XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub) {
    return 0;
}
global_variable x_input_set_state* XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

//attempt to load XInput
internal void
Win32LoadXInput() {
    HMODULE XInputLibrary = LoadLibrary("xinput1_3.dll");
    if(XInputLibrary) {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
    }
}

global_variable bool running;


struct win32_offscreen_buffer {
    BITMAPINFO Info;
    void * Memory;
    int Height;
    int Width;
    int bytesPerPixel;
    int Pitch;
};

global_variable win32_offscreen_buffer GlobalBackBuffer;

struct win32_window_dimension {
    int Width;
    int Height;
};

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
RenderWeirdGradient(win32_offscreen_buffer buffer, int xOffset, int yOffset) {
    int pitch = buffer.Width * buffer.bytesPerPixel;
    uint8 *row = (uint8 *)buffer.Memory;
    for(int y = 0; y < buffer.Height; ++y) {
        uint32 *pixel = (uint32 *)row;
        for(int x = 0; x < buffer.Width; ++x) {
            uint8 r = (y+yOffset);
            uint8 g = 0;
            uint8 b = (x+xOffset);
            //writes, then increments
            *pixel++ = ((r<<16)|(g<<8)|b);
        }
        row += pitch;
    }
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
    buffer->Memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    buffer->Pitch = buffer->Width * buffer->bytesPerPixel;
}

internal void
Win32DisplayBufferToWindow(HDC deviceContext, int WindowWidth, int WindowHeight,
                           win32_offscreen_buffer buffer) {
    StretchDIBits(deviceContext,
                  0, 0, WindowWidth, WindowHeight,
                  0, 0, buffer.Width, buffer.Height,
                  buffer.Memory,
                  &buffer.Info,
                  DIB_RGB_COLORS, SRCCOPY);


}

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
                WindowDimension.Height, GlobalBackBuffer);

            EndPaint(windowHandle, &Paint);
        } break;
        case WM_SYSKEYDOWN: {} break;
        case WM_SYSKEYUP: {} break;
        case WM_KEYDOWN: {} break;
        case WM_KEYUP: {
            uint32 VirtualKeyCode = WParam;
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
    Win32LoadXInput();

    WNDCLASS windowClass = {};
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = Win32MainWindowCallback;
    windowClass.hInstance = instance;
    //windowClass.hIcon;
    windowClass.lpszClassName = "CrestWindowClass";
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
            running = true;
            int xOffset = 0;
            int yOffset = 0;
            HDC deviceContext = GetDC(windowHandle);
            while(running){
                //Process Windows Messages
                MSG message;
                while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
                    if(message.message == WM_QUIT) running = false;

                    BOOL messageResult = GetMessage(&message, 0, 0, 0);

                    TranslateMessage(&message);
                    DispatchMessageA(&message);
                }

                //Process Controller Input
                for(int i = 0; i < XUSER_MAX_COUNT; ++i) {
                    XINPUT_STATE ControllerState;
                    if(XInputGetState(i, &ControllerState) == ERROR_SUCCESS) {
                        //Controller plugged in
                        XINPUT_GAMEPAD * pad = &ControllerState.Gamepad;
                        bool dPadUp = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                        bool dPadDown = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                        bool dPadLeft = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                        bool dPadRight = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                        bool dPadLShoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                        bool dPadRShoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                        bool AButton = (pad->wButtons & XINPUT_GAMEPAD_A);
                        bool BButton = (pad->wButtons & XINPUT_GAMEPAD_B);
                        bool XButton = (pad->wButtons & XINPUT_GAMEPAD_X);
                        bool YButton = (pad->wButtons & XINPUT_GAMEPAD_Y);

                        int16 lStickHorizontal = (pad->sThumbLX);
                        int16 lStickVertical = (pad->sThumbLY);
                    }
                    else {
                        //Controller not plugged in
                    }

                }
                RenderWeirdGradient(GlobalBackBuffer, xOffset, yOffset);

                win32_window_dimension WindowDimension = Win32GetWindowDimension(windowHandle);

                Win32DisplayBufferToWindow(deviceContext, WindowDimension.Width,
                    WindowDimension.Height, GlobalBackBuffer);


            }
        } else {
            //TODO log failure
        }
    } else {
        //TODO log failure
    }
    return 0;
}
