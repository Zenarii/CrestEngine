#define EDITSTATE_EDITED_MESH (1<<0)
#define EDITSTATE_EDITED_COLLISIONS (1<<1)
#define EDITSTATE_EDITED_WATER (1<<2)
#define EDITSTATE_EDITED_FEATURES (1<<3)

enum EditorColour {
    #define HexColour(name, upper_name, r, g, b) EDITOR_COLOUR_##upper_name,
    #include "HexColours.inc"

    EDITOR_COLOUR_COUNT
};

global char * EditorColourString[] = {
    #define HexColour(name, upper_name, r, g, b) #name ,
    #include "HexColours.inc"
};

global struct {
    b32 ShowCollisions;
    b32 ShowLargeCollisions;
    b32 ShowUI;
} EditorStateDebug;

enum {
    EDIT_MODE_TERRAIN,
    EDIT_MODE_TERRAIN_FEATURES,

    EDIT_MODE_SAVING,
    EDIT_MODE_LOADING,
    EDIT_MODE_NEW_MAP,
    EDIT_MODE_COUNT
};


typedef struct hex_edit_settings hex_edit_settings;
struct hex_edit_settings {
    i32 EditMode;
    //EDIT_MODE_TERRAIN
    b32 EditColour;
    i32 ColourIndex;
    b32 EditElevation;
    i32 Elevation;

    b32 EditWater;
    i32 WaterLevel;
    //EDIT_MODE_TERRAIN_FEATURES
    hex_feature_type EditFeature;
    i32 FeatureDensity;

    //all
    i32 BrushSize;
};


typedef struct editor_state editor_state;
struct editor_state {
    camera Camera;
    hex_edit_settings Settings;
};
