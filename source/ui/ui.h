#define CREST_UI_MAX 256
#define GENERIC_ID(x) CrestUIIDInit(__LINE__, x)

typedef enum CrestUIType {
    CREST_UI_BUTTON,
    CREST_UI_SLIDER,
} CrestUIType;


typedef struct CrestUIID {
    u32 Primary;
    u32 Secondary;
} CrestUIID;


typedef struct CrestUIWidget {
    CrestUIID id;
    CrestUIType Type;
    v4 rect;
    char Text[32];
} CrestUIWidget;

typedef struct CrestUIInput {
    r32 MouseX;
    r32 MouseY;
    b32 LeftMouseDown;
    b32 RightMouseDown;
} CrestUIInput;

typedef struct CrestUI {
    r32 MouseX;
    r32 MouseY;
    b32 LeftMouseDown;
    b32 RightMouseDown;

    u32 Count;
    CrestUIWidget Widgets[CREST_UI_MAX];

    CrestUIID hot;
    CrestUIID active;
} CrestUI;
