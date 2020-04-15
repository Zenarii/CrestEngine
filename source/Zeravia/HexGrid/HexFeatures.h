//CHUNK_WIDTH * CHUNK_HEIGHT * MAX CHUNKS if you change this must
#define MAX_FEATURE_SET_SIZE (35 * 16 * 6)
#define FEATURES_NOISE_SCALE 8.f
#define FEATURES_NUDGE_STRENGTH 0.1f

typedef enum hex_feature_type hex_feature_type;
enum hex_feature_type {
    #define HexFeature(Name) HEX_FEATURE_##Name,
    #include "HexFeatures.inc"

    HEX_FEATURE_COUNT
};

//Note(Zen): This assumes that a features .obj file name is
//the same as used in the HexFeatures.inc file
global char * HexFeaturePaths[] = {
    #define HexFeature(Name) "../assets/FeatureModels/" #Name ".obj",
    #include "HexFeatures.inc"
};

typedef struct hex_feature hex_feature;
struct hex_feature {
    u32 MeshVertices;
    matrix Model[MAX_FEATURE_SET_SIZE];
};

typedef struct hex_feature_set hex_feature_set;
struct hex_feature_set {
    u32 Shader;
    u32 VAOs[HEX_FEATURE_COUNT];
    u32 VBOs[HEX_FEATURE_COUNT];
    u32 InstancedVBOs[HEX_FEATURE_COUNT];

    hex_feature Features[HEX_FEATURE_COUNT];
};
