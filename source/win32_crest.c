//################
//# Zenarii 2020 #
//################
#include <windows.h>
#include <stdio.h>
#include "program_options.h"
#include "language_layer.h"

#include "win32_crest.h"

#include "win32_opengl.c"
#include "platform.h"
#include "app.c"

global Platform GlobalPlatform;
global b32 OpenGLHasLoaded;

//TODO(Zen): Need to sort this out later on
internal char *
CrestLoadFileAsString(const char* Path) {
    //TODO(Zen): Check if need to change security attributes
    HANDLE FileHandle = CreateFile(Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    char * Buffer;
    if(FileHandle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER FileSize;
        GetFileSizeEx(FileHandle, &FileSize);

        Buffer = (char *)malloc(FileSize.QuadPart + 16);
        DWORD BytesRead = 0;
        ReadFile(FileHandle, Buffer, FileSize.QuadPart-1, &BytesRead, NULL);
    }
    else {
        DWORD error = GetLastError();
        //TODO(Zen): Read file and log issue
        OutputDebugStringA("Failed To load file");
    }
    CloseHandle(FileHandle);
    return Buffer;
}



LRESULT CALLBACK Win32WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;
    if (message == WM_DESTROY) {
        // If we receive the destroy message, then quit the program.
        PostQuitMessage(0);
    }
    else if(message == WM_KEYDOWN || message == WM_KEYUP) {
        b32 IsDown = (message == WM_KEYDOWN);
        u32 KeyCode = wParam;
        u32 KeyIndex = 0;

        if(KeyCode == 'A') {
            KeyIndex = KEY_A;
        }
        GlobalPlatform.KeyDown[KeyIndex] = IsDown;
    }
    else if(message == WM_SIZE) {
        RECT ClientRect = {0};
        GetClientRect(window, &ClientRect);
        i32 width = ClientRect.right - ClientRect.left;
        i32 height = ClientRect.bottom - ClientRect.top;
        Win32OpenGLResize(width, height);
        GlobalPlatform.ScreenWidth = (r32)width;
        GlobalPlatform.ScreenHeight = (r32)height;
    }
    else {
        // We don't handle this message. Use the default handler.
        result = DefWindowProc(window, message, wParam, lParam);
    }
    return result;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE previousInstance,
                     LPSTR commandLine, int commandShow) {
    // Create the window class.
    WNDCLASS windowClass = {0};
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = Win32WindowProcedure;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = WINDOW_TITLE "Class";
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    RegisterClass(&windowClass);

    // Create and show the actual window.
    HWND window = CreateWindow(windowClass.lpszClassName, WINDOW_TITLE, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, NULL, NULL, instance, NULL);

    if(!window) {
        DWORD error = GetLastError();
        fprintf(stderr, "ERROR: Window Creation Failed (%i)\n", error);
        goto quit;
    }


    ShowWindow(window, commandShow);
    UpdateWindow(window);

    //Note(Zenarii): Platform initialisation
    //GlobalPlatform = {0};
    {
        GlobalPlatform.PermenantStorageSize = PERMENANT_STORAGE_SIZE;
        GlobalPlatform.PermenantStorage = VirtualAlloc(0,
            GlobalPlatform.PermenantStorageSize,
            MEM_COMMIT | MEM_RESERVE,
            PAGE_READWRITE);

        if(!GlobalPlatform.PermenantStorage) {
            DWORD error = GetLastError();
            fprintf(stderr, "ERROR: Application memory failure (%i)\n", error);
            goto quit;
        }
    }

    HDC DeviceContext = GetDC(window);
    OpenGLHasLoaded = Win32OpenGLInit(DeviceContext);
    Win32OpenGlLoadAllFunctions();

    while(!GlobalPlatform.ShouldQuit) {
        // Process window messages.
        MSG message;
        while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE) > 0) {
            TranslateMessage(&message);
            DispatchMessage(&message);

            if(message.message == WM_QUIT) {
                GlobalPlatform.ShouldQuit = 1;
            }
        }

        //Note(Zen): Get mouse coordinates
        {
            POINT MousePosition = {0};
            GetCursorPos(&MousePosition);
            ScreenToClient(window, &MousePosition);
            //Note(Zen): Using bottom-left of screen as (0,0) so need to flip y coordinate of mouse
            MousePosition.y = GlobalPlatform.ScreenHeight - MousePosition.y;
            GlobalPlatform.MouseStartX = GlobalPlatform.MouseEndX;
            GlobalPlatform.MouseStartY = GlobalPlatform.MouseEndY;
            GlobalPlatform.MouseEndX = MousePosition.x;
            GlobalPlatform.MouseEndY = MousePosition.y;
        }

        if(AppUpdate(&GlobalPlatform)) {
            GlobalPlatform.ShouldQuit = 1;
        }
        SwapBuffers(DeviceContext);
    }

    quit:;

    return 0;
}
