#define CREST_UI_MAX_COUNT 256


//Note(Zen): For maintaining state in between frames.
typedef struct CrestUIWidget {
    Type;
    v4 rect;
} CrestUIWidget;

typedef struct CrestUIID {
    u32 Primary;
    u32 Secondary;
};

typedef struct CrestUIInput {
    f32 MouseX;
    f32 MouseY;
    b32 LeftMouseDown;
    b32 RightMouseDown;
};

typedef struct CrestUI {
    f32 MouseX;
    f32 MouseY;
    b32 LeftMouseDown;
    b32 RightMouseDown;

    u32 Count;
    CrestUIWidget Widgets[UI_MAX_WIDGETS];

    CrestUIID hot;
    CrestUIID active;
} CrestUI;
