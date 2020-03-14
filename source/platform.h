enum {
    #define key(name, string) KEY_##name,
    #include "platform_key_list.inc"

    CREST_KEY_MAX
};


typedef struct Platform {
    u32 PermenantStorageSize;
    void * PermenantStorage;
    r32 ScreenHeight, ScreenWidth;
    b32 ShouldQuit;


    b32 KeyDown[CREST_KEY_MAX];
    b32 LeftMouseDown;
    b32 RightMouseDown;
    r32 MouseStartX;
    r32 MouseEndX;
    r32 MouseStartY;
    r32 MouseEndY;

    r32 TargetFPS;
    r64 TimeTakenForFrame; // time to do everything app needs
    r64 TimeTaken; //includes the time sleeping etc
} Platform;
