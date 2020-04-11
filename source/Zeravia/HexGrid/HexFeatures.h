#define MAX_FEATURE_SET_SIZE 64

typedef enum hex_feature_type hex_feature_type;
enum hex_feature_type {
    #define HexFeature(Name) HEX_FEATURE_##Name,
    #include "HexFeatures.inc"

    HEX_FEATURE_COUNT
};



typedef struct hex_feature_set hex_feature_set;
struct hex_feature_set {
    u32 Shader;
    u32 VAOs[HEX_FEATURE_COUNT];
    u32 VBOs[HEX_FEATURE_COUNT];
    u32 InstancedVBOs[HEX_FEATURE_COUNT];
    #define HexFeature(Name) matrix Name##Set[MAX_FEATURE_SET_SIZE];
    #include "HexFeatures.inc"
};
