
enum EditorColour {
    #define AddEditorColour(name, upper_name, r, g, b) EDITOR_COLOUR_##upper_name,
    #include "EditorColours.inc"

    EDITOR_COLOUR_COUNT
};

global char * EditorColourString[] = {
    #define AddEditorColour(name, upper_name, r, g, b) #name ,
    #include "EditorColours.inc"
};

global v3 EditorColourV[] = {
    #define AddEditorColour(name, upper_name, r, g, b) {r, g, b},
    #include "EditorColours.inc"
};

typedef struct hex_edit_settings hex_edit_settings;
struct hex_edit_settings {
    b32 EditColour;
    v3 Colour;
    b32 EditElevation;
    i32 Elevation;
};


typedef struct editor_state editor_state;
struct editor_state {
    camera Camera;
    hex_grid HexGrid;
    hex_edit_settings Settings;
};
