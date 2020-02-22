enum {
    #define key(name, string) KEY_##name,
    #include "platform_key_list.inc"

    CREST_KEY_MAX
};


typedef struct Platform {
    void * PermenantStorage;
    u32 PermenantStorageSize;
    r32 ScreenHeight, ScreenWidth;
    b32 ShouldQuit;
    b32 KeyDown[CREST_KEY_MAX];
    b32 LeftMouseDown;
    b32 RightMouseDown;

    r32 MouseStartX;
    r32 MouseEndX;
    r32 MouseStartY;
    r32 MouseEndY;
} Platform;
