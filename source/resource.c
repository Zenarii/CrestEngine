//TODO(Zenarii): Print error message for resources that fail to load
//Note(Zenarii): Right now assumes load successful
internal void
LoadAllResources(u32 State) {
    #define INCLUDE_SHADERS
    #define APP_RESOURCE(Name, Path, States) \
        if(!App->Shaders.##Name##.Loaded && (States & State)) { \
             App->Shaders.##Name##.ShaderID = CrestLoadShader(Path ".vs", Path ".fs", 0); \
             App->Shaders.##Name##.Loaded = 1; \
        }
    #include "app_resources.h"
}

internal void
LabelResources(app * App) {
    #define INCLUDE_SHADERS
    #define APP_RESOURCE(Name, _Path, States) \
        App->Shaders.##Name##.rType = RESOURCE_SHADER; \
        App->Shaders.##Name##.Path = _Path;
    #include "app_resources.h"
}

internal void
AppReloadResource(char * FilePath) {

    if(strncmp(FilePath, SHADER_PATH, strlen(SHADER_PATH)) == 0) {
        CrestInfoF("Reloaded File path: %s\n", FilePath);
        FilePath += strlen(SHADER_PATH);
        FilePath[strlen(FilePath)-3] = '\0'; //Clear the file extension for shaders
        CrestInfoF("Reloading Shader File: %s.\n", FilePath);

        //match shader with resource.
        resource * ShaderResource = 0;

        #define INCLUDE_SHADERS
        #define APP_RESOURCE(Name, Path, States) \
            CrestInfoF("Checking against %s\n", Path + strlen(SHADER_PATH)); \
            if(App->Shaders.##Name##.Loaded && strcmp(Path + strlen(SHADER_PATH), FilePath) == 0) { \
                 ShaderResource = &App->Shaders.##Name; \
                 CrestInfoF("Compared True\n");\
            }
        #include "app_resources.h"

        if(!ShaderResource) {
            CrestInfo("Unknown shader edited.\n");
            return;
        }

        Assert(ShaderResource->rType == RESOURCE_SHADER);

        char VertexPath[256];
        strcpy(VertexPath, SHADER_PATH);
        strcat(VertexPath, FilePath);
        strcat(VertexPath, ".vs");

        char FragmentPath[256];
        strcpy(FragmentPath, SHADER_PATH);
        strcat(FragmentPath, FilePath);
        strcat(FragmentPath, ".fs");

        b32 DidError = 0;
        u32 NewShaderID = CrestLoadShader(VertexPath, FragmentPath, &DidError);

        if(DidError) {
            CrestWarnF("Failed to compile new shader, keeping old shader.\n");
            glDeleteProgram(NewShaderID);
        }
        else if(NewShaderID == 0) {
            CrestWarnF("Invalid ShaderID for new shader, keeping old shader.\n");
            glDeleteProgram(NewShaderID);
        }
        else {
            glDeleteProgram(ShaderResource->ShaderID);
            ShaderResource->ShaderID = NewShaderID;
        }
    }
    else {
        CrestInfo("File changed not recognised as a resource path.\n");
    }
}
