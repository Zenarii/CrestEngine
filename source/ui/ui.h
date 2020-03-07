#define CREST_UI_MAX 256
#define CREST_UI_MAX_STACKED_ROWS 16
#define CREST_UI_MAX_PANELS 16
#define GENERIC_ID(x) CrestUIIDInit(__LINE__, x)

/*
TODO(Zen): Useful UI remaining
text label (centre/right/left)
text edit (?)

TODO(Zen): UI customisation
easily use different styles (still render in a single call)
padding in rows/columns

TODO(Zen): Other features
Panels on top of other panels
Figure out depth drawing.
 - Sort Triangles?
 - Give panels certain "z-indexes" and increase this for the rectangles drawn?

TODO(Zen): Testing
Test explicitly pushing multiple rows

*/

typedef struct CrestUIStyle {
    v2 Padding;
    v4 ButtonColour;
    v4 ButtonHotColour;
    v4 ButtonBorderColour;
    v4 HeaderColour;
    v4 HeaderBorderColour;
    v4 PanelColour;
} CrestUIStyle;

global CrestUIStyle DefaultStyle = {
    .Padding = {8.f, 4.f},
    .ButtonColour = {75.f/255.f, 75.f/255.f, 75.f/255.f, 1.f},
    .ButtonHotColour = {95.f/255.f, 95.f/255.f, 95.f/255.f, 1.f},
    .ButtonBorderColour = {1.0f, 1.0f, 1.0f, 1.0f},
    .HeaderColour = {0.9f, 0.3f, 0.f, 1.f},
    .HeaderBorderColour = {0.1f, 0.1f, 0.2f, 1.0f},
    .PanelColour = {0.1f, 0.1f, 0.1f, 0.6f}
};

typedef enum CrestUIType {
    CREST_UI_BUTTON,
    CREST_UI_SLIDER,
    CREST_UI_HEADER,
    CREST_UI_PANEL,
    CREST_UI_TEXTLABEL
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

    //TEMP(ZEN): think of a better way to implement elements overlapping
    r32 Precedence;

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
        r32 Width;
        r32 Height;
        r32 Precedence;
    } PanelStack[CREST_UI_MAX_PANELS];

    CrestUIID hot;
    CrestUIID active;
} CrestUI;
