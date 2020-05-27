//IDEA(Zenarii): Could do string interning to test the paths for hot reloading assets

typedef enum resource_type resource_type;
enum resource_type {
    RESOURCE_INVALID,
    RESOURCE_SHADER
};

typedef struct resource resource;
struct resource {
    resource_type rType;
    b32 Loaded;

    union {
        struct {
            u32 ShaderID;
            char * Path;
        }; //Shaders
    };
};

internal void
LoadAllResources(u32 State);
