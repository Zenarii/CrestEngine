#define CREST_UI_MAX 256
#define CREST_UI_MAX_STACKED_ROWS 16
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

    union {
        r32 Value;
    }; //slider

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

    u32 AutoLayoutStackPosition;
    struct {
        v2 Position;
        v2 Size;
        r32 ProgressX;
        r32 ProgressY;
        b32 IsRow;
        u32 ElementsInRow;
        u32 MaxElementsPerRow;
    } AutoLayoutStack[CREST_UI_MAX_STACKED_ROWS];

    CrestUIID hot;
    CrestUIID active;
} CrestUI;
