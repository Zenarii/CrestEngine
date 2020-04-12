#define EDITSTATE_EDITED_MESH (1<<0)
#define EDITSTATE_EDITED_COLLISIONS (1<<1)
#define EDITSTATE_EDITED_WATER (1<<2)

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

enum {
    EDIT_MODE_TERRAIN,
    EDIT_MODE_TERRAIN_FEATURES,

    EDIT_MODE_COUNT
};


typedef struct hex_edit_settings hex_edit_settings;
struct hex_edit_settings {
    i32 EditMode;
    //EDIT_MODE_TERRAIN
    b32 EditColour;
    v3 Colour;
    b32 EditElevation;
    i32 Elevation;
    i32 BrushSize;
    //EDIT_MODE_TERRAIN_FEATURES
    b32 EditWater;
    i32 WaterLevel;
    b32 EditTrees;
    i32 FeatureDensity;
};


typedef struct editor_state editor_state;
struct editor_state {
    camera Camera;
    hex_grid HexGrid;
    hex_edit_settings Settings;
};
