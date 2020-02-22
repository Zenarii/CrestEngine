#define CREST_UI_MAX 256


typedef enum CrestUIType {
    CREST_UI_BUTTON
} CrestUIType;

//Note(Zen): For maintaining state in between frames.
typedef struct CrestUIWidget {
    CrestUIType Type;
    v4 rect;
} CrestUIWidget;

typedef struct CrestUIID {
    u32 Primary;
    u32 Secondary;
} CrestUIID;

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
