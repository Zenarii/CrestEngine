//################
//# Zenarii 2020 #
//################
#define WIN32_LEAN_AND_MEAN
#include <malloc.h>
#include <windows.h>
#include <timeapi.h>
#include <fileapi.h>

#include <stdio.h>
#include <stdlib.h>

#include "program_options.h"
#include "language_layer.h"
#include "win32_crest.h"

#include "win32_opengl.c"
#include "platform.h"
#include "app.c"

global Platform GlobalPlatform;
global char GlobalExecutableDirectory[256];
global b32 OpenGLHasLoaded;
global b32 FileWatcherThreadHasLoaded;


/*
    File IO
*/

internal char *
CrestLoadFileAsString(const char* Path) {
    //TODO(Zen): Check if need to change security attributes
    HANDLE FileHandle = CreateFile(Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    char * Buffer;
    if(FileHandle != INVALID_HANDLE_VALUE) {
        DWORD FileSize = GetFileSize(FileHandle, 0);

        Buffer = malloc(FileSize+1);
        DWORD BytesRead = 0;
        ReadFile(FileHandle, Buffer, FileSize, &BytesRead, NULL);
        Buffer[BytesRead-1] = 0;
    }
    else {
        DWORD error = GetLastError();
        //TODO(Zen): Read file and log issue
        char output[128];
        sprintf(output, "Failed to load file: %s\n", Path);
        OutputDebugStringA(output);
        return 0;
    }
    CloseHandle(FileHandle);
    return Buffer;
}
//Note(Zen): Directory writing to must exist.
//TODO(ZEN): Either Create missing directories or fail
internal void
CrestWriteFile(const char * Path, const char * Data, i32 DataLength) {
    HANDLE FileHandle = CreateFile(Path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);


    i32 Success = WriteFile(FileHandle, Data, DataLength, 0, 0);
    CloseHandle(FileHandle);
    if(!Success) {
        i32 Error = GetLastError();
    }

}

internal b32
CrestDoesFileExist(const char * Path) {
    WIN32_FIND_DATA FindFileData;
    HANDLE handle = FindFirstFile(Path, &FindFileData) ;
    b32 found = handle != INVALID_HANDLE_VALUE;
    if(found) {
        FindClose(handle);
    }
    return found;
}

internal void
CrestMakeDirectory(const char * Path) {
    CreateDirectoryA(Path, 0);
}

/*
    Timing
*/
global LARGE_INTEGER Win32ClockFrequency;
global LARGE_INTEGER Win32GameStartTime;
internal r64
CrestCurrentTime() {
    LARGE_INTEGER Now;
    QueryPerformanceCounter(&Now);
    return ((r64)(Now.QuadPart - Win32GameStartTime.QuadPart))/((r64)Win32ClockFrequency.QuadPart);
}

/*
    Threads
*/

DWORD WINAPI
Win32FileWatcherProc(LPVOID Paramaters) {
    OutputDebugStringA("[Threading] Starting file watcher.\n");

    HDC DeviceContext = *(HDC *)Paramaters;

    //Note(Zen) Load OpenGL Context for this thread
    int ContextAttributes[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        0
    };

    wglMakeCurrent(DeviceContext, GlobalOpenGLContext);
    HGLRC FileWatcherContext = wglCreateContextAttribsARB(DeviceContext, GlobalOpenGLContext, ContextAttributes);
    wglMakeCurrent(DeviceContext, FileWatcherContext);

    if(!FileWatcherContext) {
        OutputDebugStringA("Failed to load OpenGL context for file watcher thread");
    }

    FileWatcherThreadHasLoaded = 1;

    //Note(Zen): Watch all files and subfolders from the exe.
    HANDLE Directory = CreateFileA(".", FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, 0);

    DWORD BytesRead = 0;
    //Note(Zen): This functions as a single FILE_NOTIFY_INFORMATION struct.
    //    Due to needing a buffer to store the file name string need something larger than a single
    //    FNI struct, and using a char buffer doesn't seem to work, likely due to the pointer in the struct.
    //    Since I don't want to malloc here, this works as a solution.
    FILE_NOTIFY_INFORMATION ChangedFileInfo[16] = {0};
    for(;;) {
        BOOL Success = ReadDirectoryChangesW(Directory, ChangedFileInfo, sizeof(ChangedFileInfo), TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE, &BytesRead, 0, 0);

        if(Success && BytesRead) {
            char FileName[256] = {0};
            wcstombs(FileName, ChangedFileInfo->FileName, ChangedFileInfo->FileNameLength);

            if(FileName) {
                char Output[256] = {0};
                sprintf(Output, "[File Watcher] Reloading file %s\n", FileName);
                OutputDebugStringA(Output);
                AppReloadResource(FileName);
            }
        }
        else {
            OutputDebugStringA("[File Watcher] ReadDirectoryChangesW failed!\n");
        }
    }
    OutputDebugStringA("[Threading] Closing file watcher.\n");
    return 0;
}

/*
    Win32 Code
*/
LRESULT CALLBACK Win32WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;
    if (message == WM_DESTROY) {
        // If we receive the destroy message, then quit the program.
        PostQuitMessage(0);
    }
    else if ((message == WM_KEYDOWN) || (message == WM_KEYUP)) {
        b32 IsDown = (message == WM_KEYDOWN);
        u32 KeyCode = wParam;
        u32 KeyIndex = 0;

        if((KeyCode >= 65) && (KeyCode <= 90)) {
            KeyIndex = KEY_A + (KeyCode - 65);
        }

        if(KeyCode >= VK_LEFT && KeyCode <= VK_DOWN) {
            KeyIndex = KEY_LEFT + (KeyCode - VK_LEFT);
        }

        if(KeyCode == VK_ESCAPE) KeyIndex = KEY_ESC;
        if(KeyCode == VK_CONTROL) KeyIndex = KEY_CTRL;
        if(KeyCode == VK_TAB) KeyIndex = KEY_TAB;
        if(KeyCode == VK_RETURN) KeyIndex = KEY_RETURN;
        if(KeyCode == VK_SHIFT) KeyIndex = KEY_SHIFT;

        if(KeyCode >= VK_F1 && KeyCode <= VK_F12) KeyIndex = KEY_F1 + (KeyCode - VK_F1);

        GlobalPlatform.KeyDown[KeyIndex] = IsDown;
    }
    else if(message == WM_CHAR) {
        AppPutChar(wParam);
    }
    else if (message == WM_LBUTTONDOWN || message == WM_LBUTTONUP) {
        b32 IsDown = (message == WM_LBUTTONDOWN);
        GlobalPlatform.LeftMouseDown = IsDown;
    }
    else if (message == WM_RBUTTONDOWN || message == WM_RBUTTONUP) {
        b32 IsDown = (message == WM_RBUTTONDOWN);
        GlobalPlatform.RightMouseDown = IsDown;
    }
    else if(message == WM_MOUSEWHEEL) {
        i32 MouseDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        //Note(Zen): Get Mouse Scroll between 0 and 1
        GlobalPlatform.MouseScroll = MouseDelta / 120.f;
    }
    else if (message == WM_SIZE) {
        RECT ClientRect = {0};
        GetClientRect(window, &ClientRect);
        i32 width = ClientRect.right - ClientRect.left;
        i32 height = ClientRect.bottom - ClientRect.top;
        Win32OpenGLResize(width, height);
        if(App && App->Initialised) {
            AppResize(width, height);
        }
        GlobalPlatform.ScreenWidth = (r32)width;
        GlobalPlatform.ScreenHeight = (r32)height;
    }
    else {
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
        char Buffer[256];
        sprintf(Buffer, "ERROR: Window Creation Failed (%i)\n", error);

        OutputDebugStringA(Buffer);
        goto quit;
    }

    //Get executable directory
    {
        GetCurrentDirectory(256, GlobalExecutableDirectory);
        OutputDebugStringA("Executable Directory is: ");
        OutputDebugStringA(GlobalExecutableDirectory);
        OutputDebugStringA("\n");
    }

    ShowWindow(window, commandShow);
    UpdateWindow(window);

    //Note(Zenarii): Platform initialisation
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

    //Note(Zen): Graphics initialisation
    HDC DeviceContext = GetDC(window);
    OpenGLHasLoaded = Win32OpenGLInit(DeviceContext);
    Win32OpenGlLoadAllFunctions();


    //Note(Zen): Load the file watcher proc.
    wglMakeCurrent(DeviceContext, 0);
    CreateThread(0, 0, Win32FileWatcherProc, &DeviceContext, 0, 0);
    //Note(Zen): wait for new thread to create own OpenGL Context.
    while(!FileWatcherThreadHasLoaded);
    wglMakeCurrent(DeviceContext, GlobalOpenGLContext);
    OutputDebugStringA("[Threading] Loaded Win32 Threads.\n");


    //Note(Zen): Setup timing
    UINT DesiredSleepGranularity = 1;
    BOOL SetSleepGranular = (timeBeginPeriod(DesiredSleepGranularity) == TIMERR_NOERROR);
    QueryPerformanceFrequency(&Win32ClockFrequency);

    LARGE_INTEGER StartTime = {0};
    QueryPerformanceCounter(&StartTime);
    Win32GameStartTime = StartTime;
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

            GlobalPlatform.MouseStartX = GlobalPlatform.MouseEndX;
            GlobalPlatform.MouseStartY = GlobalPlatform.MouseEndY;
            GlobalPlatform.MouseEndX = MousePosition.x;
            GlobalPlatform.MouseEndY = MousePosition.y;

        }

        if(AppUpdate(&GlobalPlatform)) {
            GlobalPlatform.ShouldQuit = 1;
        }
        SwapBuffers(DeviceContext);

        //Note(Zen): Timing
        LARGE_INTEGER EndTime;
        QueryPerformanceCounter(&EndTime);

        r64 TimeTaken = ((r64)(EndTime.QuadPart - StartTime.QuadPart))/((r64)Win32ClockFrequency.QuadPart);
        GlobalPlatform.TimeTakenForFrame = TimeTaken;
        while (TimeTaken < 1.f/GlobalPlatform.TargetFPS) {
            if(SetSleepGranular) {
                DWORD TimeToWait = (DWORD) (((1.f/GlobalPlatform.TargetFPS)-TimeTaken) * 1000);
                if(TimeToWait > 0) {
                    Sleep(TimeToWait);
                }
            }

            QueryPerformanceCounter(&EndTime);
            TimeTaken = ((r64)(EndTime.QuadPart - StartTime.QuadPart))/((r64)Win32ClockFrequency.QuadPart);
        }
        GlobalPlatform.TimeTaken = TimeTaken;
        StartTime = EndTime;

        //Clear MouseScroll
        GlobalPlatform.MouseScroll = 0;
    }

    quit:;

    return 0;
}
