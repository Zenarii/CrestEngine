/*
    A header file that is designed to be included multiple times
    per resource

    APP_RESOURCE(Name, Path, States)
*/

#define SHADER_PATH "assets/shaders/"

#ifdef INCLUDE_SHADERS
APP_RESOURCE(Water, SHADER_PATH "hex_water_shader", APP_STATE_GAME | APP_STATE_EDITOR)
APP_RESOURCE(Terrain, SHADER_PATH "hex_shader", APP_STATE_GAME | APP_STATE_EDITOR)
APP_RESOURCE(Feature, SHADER_PATH "hex_feature_shader", APP_STATE_GAME | APP_STATE_EDITOR)
#undef INCLUDE_SHADERS
#endif


#undef APP_RESOURCE
