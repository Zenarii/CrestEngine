//TODO(Zenarii): Print error message for resources that fail to load
//Note(Zenarii): Right now assumes load successful
internal void
LoadAllResources(u32 State) {
    #define INCLUDE_SHADERS
    #define APP_RESOURCE(Name, Path, States) \
        if(!App->Shaders.##Name##.Loaded && (States & State)) { \
             App->Shaders.##Name##.ShaderID = CrestLoadShader(Path ".vs", Path ".fs"); \
             App->Shaders.##Name##.Loaded = 1; \
        }
    #include "app_resources.h"
}

internal void
LabelResources(app * App) {
    #define INCLUDE_SHADERS
    #define APP_RESOURCE(Name, Path, States) App->Shaders.##Name##.rType = RESOURCE_SHADER;
    #include "app_resources.h"
}
