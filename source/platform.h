enum {
    #define key(name, string) KEY_##name,
    #include "platform_key_list.inc"

    CREST_KEY_MAX
};


typedef struct Platform {
    void * PermenantStorage;
    u32 PermenantStorageSize;
    b32 ShouldQuit;
    b32 KeyDown[CREST_KEY_MAX];
} Platform;
