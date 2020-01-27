//Zenarii 2020
#include <windows.h>
#include <stdint.h>
#include <xinput.h>
#include <dsound.h>


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

global_variable bool running;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;


//XINPUT stubs
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub) {
    return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_get_state* XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub) {
    return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_set_state* XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

//attempt to load XInput
internal void
Win32LoadXInput() {
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if(!XInputLibrary) {
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }
    //TODO inform which xinput library is being used
    if(XInputLibrary) {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        if(!XInputGetState) XInputGetState = XInputGetStateStub;
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
        if(!XInputSetState) XInputSetState = XInputSetStateStub;
    }
}

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal void
Win32InitDirectSound(HWND windowHandle, int32 SamplesPerSecond, int32 BufferSize) {
    //load the library
    HMODULE DirectSoundLibrary = LoadLibraryA("dsound.dll");

    if(DirectSoundLibrary) {
        //get a directsound object
        direct_sound_create * DirectSoundCreate = (direct_sound_create *)
            GetProcAddress(DirectSoundLibrary, "DirectSoundCreate");


        LPDIRECTSOUND DirectSound;
        if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0))) {

            WAVEFORMATEX WaveFormat = {};
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = SamplesPerSecond;
            WaveFormat.wBitsPerSample = 16;
            WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample)/8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
            WaveFormat.cbSize = 0;

            if(SUCCEEDED(DirectSound->SetCooperativeLevel(windowHandle, DSSCL_PRIORITY))) {
                //Create Primary Buffer
                DSBUFFERDESC BufferDescription = {};
                //should game sound keep playing? DSBCAPS_GLOBALFOCUS
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0))) {
                    if(SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat))) {
                        OutputDebugStringA("Primary Buffer Format Set");
                    }
                    else {
                        OutputDebugStringA("Failed to set primary Buffer Format Set");
                    }
                }
            }
            else {

            }
            DSBUFFERDESC BufferDescription = {};
            BufferDescription.dwSize = sizeof(BufferDescription);
            //BufferDescription.dwFlags = DS  ;
            BufferDescription.dwBufferBytes = BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;
            //create secondary buffer
            if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0))) {
                OutputDebugStringA("Secondary Buffer Format Set");
            }
        }
        else {
            //TODO diagnostic
        }
    }
    else {
        //TODO: log this
    }
}





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
RenderWeirdGradient(win32_offscreen_buffer * buffer, int xOffset, int yOffset) {
    int pitch = buffer->Width * buffer->bytesPerPixel;
    uint8 *row = (uint8 *)buffer->Memory;
    for(int y = 0; y < buffer->Height; ++y) {
        uint32 *pixel = (uint32 *)row;
        for(int x = 0; x < buffer->Width; ++x) {
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
                WindowDimension.Height, &GlobalBackBuffer);

            EndPaint(windowHandle, &Paint);
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
    Win32LoadXInput();

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
            HDC deviceContext = GetDC(windowHandle);

            int SamplesPerSecond = 48000;
            int Hz = 256;
            uint32 RunningSampleIndex = 0;
            int SquareWavePeriod = SamplesPerSecond/Hz;
            int HalfSquareWavePeriod = SquareWavePeriod/2;
            int BytesPerSample = sizeof(int16) * 2;
            int SecondaryBufferSize = SamplesPerSecond * BytesPerSample;

            Win32InitDirectSound(windowHandle, SamplesPerSecond, SecondaryBufferSize);
            GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
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

                RenderWeirdGradient(&GlobalBackBuffer, xOffset, yOffset);
                DWORD PlayCursor;
                DWORD WriteCursor;
                if(SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor))) {


                    //test for directsound
                    DWORD WritePointer;
                    DWORD BytesToWrite;
                    VOID * Region1;
                    DWORD Region1Size;
                    VOID * Region2;
                    DWORD Region2Size;

                    DWORD ByteToLock = (RunningSampleIndex*BytesPerSample) % SecondaryBufferSize;

                    if (ByteToLock > PlayCursor) {
                        BytesToWrite = SecondaryBufferSize - ByteToLock;
                        BytesToWrite += PlayCursor;
                    }
                    else {
                        BytesToWrite = PlayCursor - ByteToLock;
                    }

                    GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite,
                                                &Region1, &Region1Size,
                                                &Region2, &Region2Size,
                                                0);

                    DWORD Region1SampleCount = Region1Size/BytesPerSample;
                    DWORD Region2SampleCount = Region2Size/BytesPerSample;
                    int16 * SampleOut = (int16 *)Region1;
                    for(DWORD SampleIndex = 0; SampleIndex < Region1Size; ++SampleIndex) {
                        int16 SampleValue = ((RunningSampleIndex / HalfSquareWavePeriod)%2) ? 16000 : -16000;
                        *SampleOut++ = SampleValue;
                        *SampleOut++ = SampleValue;
                        ++RunningSampleIndex;
                    }
                    SampleOut = (int16 *)Region2;
                    for(DWORD SampleIndex = 0; SampleIndex < Region2Size; ++SampleIndex) {
                        int16 SampleValue = ((RunningSampleIndex / HalfSquareWavePeriod)%2) ? 16000 : -16000;
                        *SampleOut++ = SampleValue;
                        *SampleOut++ = SampleValue;
                        ++RunningSampleIndex;
                    }

                }
                win32_window_dimension WindowDimension = Win32GetWindowDimension(windowHandle);

                Win32DisplayBufferToWindow(deviceContext, WindowDimension.Width,
                    WindowDimension.Height, &GlobalBackBuffer);


            }
        } else {
            //TODO log failure
        }
    } else {
        //TODO log failure
    }
    return 0;
}
