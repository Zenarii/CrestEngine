#define CREST_UI_MAX 256
#define CREST_UI_MAX_STACKED_ROWS 16
#define CREST_UI_MAX_PANELS 16
#define GENERIC_ID(x) CrestUIIDInit(__LINE__, x)

/*
TODO(Zen): Useful UI remaining
text label (centre/right/left)
text edit (?)
Some kind of panel feature with drag and drop

TODO(Zen): UI customisation
easily use different styles (still render in a single call)
padding in rows/columns

TODO(Zen): Other features
Panels on top f other panels
*/



typedef enum CrestUIType {
    CREST_UI_BUTTON,
    CREST_UI_SLIDER,
    CREST_UI_HEADER,
    CREST_UI_PANEL
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
    r32 MouseStartX;
    r32 MouseStartY;
    b32 LeftMouseDown;
    b32 RightMouseDown;
} CrestUIInput;

typedef struct CrestUI {
    r32 MouseX;
    r32 MouseY;
    r32 MouseStartX;
    r32 MouseStartY;
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


    u32 PanelStackPosition;
    struct {
        v2 Position;
        u32 Rows;
    } PanelStack[CREST_UI_MAX_PANELS];

    CrestUIID hot;
    CrestUIID active;
} CrestUI;
